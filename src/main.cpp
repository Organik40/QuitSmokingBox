#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <time.h>
#include "config.h"
#include "display.h"
#include "servo_control.h"
#include "timer.h"
#include "button.h"
#include <AsyncWebSocket.h>
#include <HTTPClient.h>

// Global objects
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
Preferences preferences;
Display display;
ServoControl servoControl;
Timer timer;
Button button;

// AI Emergency Gatekeeper
struct EmergencySession {
    bool active = false;
    unsigned long startTime = 0;
    String sessionId = "";
    int messageCount = 0;
    String trigger = "";
    String personality = "supportive";
    bool reflectionMode = false;
    int currentReflectionQuestion = 0;
    String reflectionResponses[5] = {"", "", "", "", ""};
    bool reflectionCompleted = false;
};

struct ReflectionSession {
    String trigger;
    String personality;
    int currentQuestion;
    unsigned long startTime;
    String responses[5];
    bool completed;
};

EmergencySession currentEmergencySession;
ReflectionSession currentReflection;

// Global variables
BoxState currentState = SETUP;
TimerMode currentMode = FIXED_INTERVAL;
unsigned long lastDisplayUpdate = 0;
unsigned long lastStatusSave = 0;
bool wifiConnected = false;

// Multi-language support
struct LanguageConfig {
  String currentLanguage = "en";
  String supportedLanguages = "en,pt,es,fr,de";
};

struct CostConfig {
  String productName = "Cigarettes";
  String currency = "EUR";
  bool usePackPrice = false;
  float cigaretteCost = 0.50;
  float packCost = 10.00;
  int cigarettesPerPack = 20;
};

LanguageConfig languageConfig;
CostConfig costConfig;

// Function declarations
void setupWiFi();
void setupTimeSync();
void setupWebServer();
void setupHardware();
void handleWebRequests();
void updateDisplay();
void saveStatus();
void loadConfiguration();
String getStatusJSON();
void transitionToState(BoxState newState);
// AI Emergency Gatekeeper functions
bool isEmergencyAllowedOnCurrentNetwork();
void startEmergencySession(String trigger);
String getAIResponse(String userMessage, String trigger, String personality);
String getSimpleAIResponse(String userMessage, String trigger, String personality);
String getEnhancedAIResponse(String userMessage, String trigger, String personality, int messageCount);
String getWelcomeMessage(String personality);
String getTriggerAnalysis(String userMessage, String trigger, String personality);
String getReflectionQuestion(String trigger, int questionNumber);
String analyzeReflectionResponse(String userMessage, String trigger, String personality);
String getCopingStrategy(String trigger, String personality);
String getEncouragementAndAlternatives(String userMessage, String trigger, String personality);
String getOpenAIResponse(String userMessage, String trigger, String personality);
String getLocalAIResponse(String userMessage, String trigger, String personality);
// Reflection system functions
void startReflectionSession(String trigger, String personality);
String getNextReflectionQuestion();
void recordReflectionResponse(String response);
String generateReflectionSummary();
bool isReflectionSessionActive();
void broadcastStatus();
void updateStatistics();
// Timer mode implementations
void updateGradualReduction();
void updateCompleteQuitMode();

void setup() {
    Serial.begin(115200);
    Serial.println("🚭 Quit Smoking Timer Box - Starting...");
    
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("❌ SPIFFS initialization failed!");
        return;
    }
    
    // Initialize preferences
    preferences.begin(PREF_NAMESPACE, false);
    
    // Setup hardware
    setupHardware();
    
    // Load saved configuration
    loadConfiguration();
    
    // Setup WiFi
    setupWiFi();
    
    // Setup web server
    setupWebServer();
    
    // Initialize display with welcome message
    display.showWelcome();
    delay(2000);
    
    // Start in locked state if timer is active, otherwise unlocked
    if (timer.isActive()) {
        transitionToState(LOCKED);
        servoControl.lock();
    } else {
        transitionToState(UNLOCKED);
        servoControl.unlock();
    }
    
    Serial.println("✅ Quit Smoking Timer Box - Ready!");
    Serial.print("📱 Web interface: http://");
    Serial.println(WiFi.softAPIP());
}

void loop() {
    // Update button state
    button.update();
    
    // Handle button press (emergency unlock from inside)
    if (button.wasPressed() && currentState == LOCKED) {
        Serial.println("🚨 Emergency button pressed!");
        
        // Check emergency unlock limits
        int emergencyCount = preferences.getInt(KEY_EMERGENCY_COUNT, 0);
        if (emergencyCount < MAX_EMERGENCY_UNLOCKS_PER_DAY) {
            // Perform emergency unlock
            preferences.putInt(KEY_EMERGENCY_COUNT, emergencyCount + 1);
            
            // Add penalty to next timer
            int currentInterval = preferences.getInt(KEY_INTERVAL_MINUTES, DEFAULT_TIMER_MINUTES);
            int newInterval = currentInterval + EMERGENCY_UNLOCK_PENALTY;
            preferences.putInt(KEY_INTERVAL_MINUTES, newInterval);
            
            // Unlock and reset timer
            timer.stop();
            transitionToState(UNLOCKED);
            servoControl.unlock();
            
            Serial.printf("Emergency unlock granted. Penalty: %d minutes added to next timer.\n", EMERGENCY_UNLOCK_PENALTY);
        } else {
            Serial.println("❌ Maximum emergency unlocks per day reached!");
            display.showMessage("Emergency limit reached!", 2000);
        }
    }
    
    // Update timer
    timer.update();
    
    // Check if timer finished
    if (timer.wasTriggered()) {
        Serial.println("⏰ Timer finished - Unlocking box");
        transitionToState(UNLOCKED);
        servoControl.unlock();
        
        // Increment cigarette count
        int totalCigs = preferences.getInt(KEY_TOTAL_CIGARETTES, 0);
        preferences.putInt(KEY_TOTAL_CIGARETTES, totalCigs + 1);
        
        // Update last unlock time
        preferences.putULong64(KEY_LAST_UNLOCK, millis());
        
        // Apply timer mode specific logic
        TimerMode currentMode = (TimerMode)preferences.getInt(KEY_TIMER_MODE, FIXED_INTERVAL);
        if (currentMode == GRADUAL_REDUCTION) {
            updateGradualReduction();
        } else if (currentMode == COMPLETE_QUIT) {
            updateCompleteQuitMode();
        }
        
        // Update statistics
        updateStatistics();
        
        // Broadcast status update to WebSocket clients
        broadcastStatus();
    }
    
    // Update display periodically
    if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = millis();
        
        // Broadcast status every 5 seconds when clients are connected
        static unsigned long lastBroadcast = 0;
        if (ws.count() > 0 && millis() - lastBroadcast >= 5000) {
            broadcastStatus();
            lastBroadcast = millis();
        }
    }
    
    // Save status and update statistics periodically
    if (millis() - lastStatusSave >= 60000) { // Every minute
        saveStatus();
        updateStatistics();
        lastStatusSave = millis();
    }
    
    // Reset emergency count daily (simplified - resets after 24 hours of uptime)
    static unsigned long lastReset = 0;
    if (millis() - lastReset >= 86400000) { // 24 hours
        preferences.putInt(KEY_EMERGENCY_COUNT, 0);
        lastReset = millis();
        Serial.println("🔄 Daily emergency count reset");
    }
    
    delay(50); // Small delay to prevent watchdog issues
}

void setupHardware() {
    Serial.println("🔧 Initializing hardware...");
    
    // Initialize I2C for OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    
    // Initialize display
    if (!display.begin()) {
        Serial.println("❌ OLED display initialization failed!");
        while (1) delay(100);
    }
    
    // Initialize servo
    servoControl.begin();
    
    // Initialize timer
    timer.begin();
    
    // Initialize button
    button.begin();
    
    Serial.println("✅ Hardware initialized");
}

void setupWiFi() {
    Serial.println("📶 Setting up WiFi...");
    
    // Try to connect to stored WiFi credentials first
    String storedSSID = preferences.getString("wifi_ssid", "");
    String storedPassword = preferences.getString("wifi_password", "");
    
    if (storedSSID.length() > 0) {
        Serial.printf("🔄 Attempting to connect to stored WiFi: %s\n", storedSSID.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(storedSSID.c_str(), storedPassword.c_str());
        
        // Wait up to 15 seconds for connection
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 30) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n✅ Connected to WiFi!");
            Serial.printf("📱 IP Address: %s\n", WiFi.localIP().toString().c_str());
            wifiConnected = true;
            setupTimeSync();
            return;
        } else {
            Serial.println("\n❌ Failed to connect to stored WiFi");
        }
    }
    
    // Fall back to Access Point mode
    Serial.println("📶 Starting WiFi Access Point...");
    WiFi.mode(WIFI_AP);
    
    bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    if (apStarted) {
        IPAddress ip = WiFi.softAPIP();
        Serial.printf("✅ WiFi AP started: %s\n", AP_SSID);
        Serial.printf("📱 IP Address: %s\n", ip.toString().c_str());
        Serial.printf("🔐 Password: %s\n", AP_PASSWORD);
        wifiConnected = true;
        
        // Setup time synchronization (NTP)
        setupTimeSync();
    } else {
        Serial.println("❌ Failed to start WiFi AP!");
        wifiConnected = false;
    }
}

void setupTimeSync() {
    Serial.println("🕒 Setting up time synchronization...");
    
    // Configure NTP
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    
    // Wait for time to be set
    int attempts = 0;
    while (time(nullptr) < 1000000000L && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (time(nullptr) > 1000000000L) {
        Serial.println("\n✅ Time synchronized with NTP");
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            Serial.printf("📅 Current time: %04d-%02d-%02d %02d:%02d:%02d\n",
                         timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                         timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }
    } else {
        Serial.println("\n⚠️ Failed to synchronize time with NTP");
    }
}

void setupWebServer() {
    Serial.println("🌐 Setting up web server...");
    
    // Serve static files from SPIFFS
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
    
    // API endpoint: Get current status
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", getStatusJSON());
    });
    
    // API endpoint: Get configuration
    server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(1024);
        
        TimerMode currentMode = (TimerMode)preferences.getInt(KEY_TIMER_MODE, FIXED_INTERVAL);
        
        doc["timerMode"] = currentMode;
        doc["intervalMinutes"] = preferences.getInt(KEY_INTERVAL_MINUTES, DEFAULT_TIMER_MINUTES);
        doc["dailyLimit"] = preferences.getInt(KEY_DAILY_LIMIT, 10);
        doc["emergencyUnlocks"] = MAX_EMERGENCY_UNLOCKS_PER_DAY;
        
        // Add schedule data for scheduled modes
        if (currentMode == DAILY_SCHEDULE || currentMode == WEEKLY_SCHEDULE || currentMode == CUSTOM_SCHEDULE) {
            doc["scheduleHour"] = preferences.getInt(KEY_DAILY_HOUR, 22);
            doc["scheduleMinute"] = preferences.getInt(KEY_DAILY_MINUTE, 0);
            doc["unlockDuration"] = preferences.getInt(KEY_UNLOCK_DURATION, 30);
            
            if (currentMode == WEEKLY_SCHEDULE) {
                doc["weekDay"] = preferences.getInt(KEY_WEEKLY_DAY, 0);
            }
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // API endpoint: Save configuration
    server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Handle POST data
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, (char*)data);
        
        // Save basic configuration
        TimerMode newMode = (TimerMode)doc["timerMode"].as<int>();
        preferences.putInt(KEY_TIMER_MODE, newMode);
        preferences.putInt(KEY_INTERVAL_MINUTES, doc["intervalMinutes"]);
        preferences.putInt(KEY_DAILY_LIMIT, doc["dailyLimit"]);
        
        // Handle schedule configuration for new modes
        if (newMode == DAILY_SCHEDULE || newMode == WEEKLY_SCHEDULE || newMode == CUSTOM_SCHEDULE) {
            if (doc.containsKey("scheduleHour") && doc.containsKey("scheduleMinute")) {
                int hour = doc["scheduleHour"];
                int minute = doc["scheduleMinute"];
                int unlockDuration = doc["unlockDuration"] | 30; // Default 30 minutes
                
                if (newMode == DAILY_SCHEDULE) {
                    timer.setDailySchedule(hour, minute, unlockDuration);
                } else if (newMode == WEEKLY_SCHEDULE && doc.containsKey("weekDay")) {
                    int weekDay = doc["weekDay"];
                    timer.setWeeklySchedule(weekDay, hour, minute, unlockDuration);
                }
                
                Serial.printf("📅 Schedule configured: %02d:%02d for %d minutes\n", hour, minute, unlockDuration);
            }
        }
        
        currentMode = newMode;
        
        DynamicJsonDocument response(256);
        response["success"] = true;
        response["message"] = "Configuration saved";
        
        String responseStr;
        serializeJson(response, responseStr);
        request->send(200, "application/json", responseStr);
        
        Serial.println("💾 Configuration updated via web interface");
    });
    
    // API endpoint: Manual unlock
    server.on("/api/unlock", HTTP_POST, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument response(256);
        
        if (currentState != UNLOCKED) {
            timer.stop();
            transitionToState(UNLOCKED);
            servoControl.unlock();
            
            response["success"] = true;
            response["message"] = "Box unlocked";
            
            Serial.println("🔓 Manual unlock via web interface");
        } else {
            response["success"] = false;
            response["message"] = "Box already unlocked";
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        request->send(200, "application/json", responseStr);
    });
    
    // API endpoint: Emergency unlock
    server.on("/api/emergency", HTTP_POST, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument response(256);
        
        int emergencyCount = preferences.getInt(KEY_EMERGENCY_COUNT, 0);
        
        if (emergencyCount < MAX_EMERGENCY_UNLOCKS_PER_DAY) {
            preferences.putInt(KEY_EMERGENCY_COUNT, emergencyCount + 1);
            
            // Add penalty
            int currentInterval = preferences.getInt(KEY_INTERVAL_MINUTES, DEFAULT_TIMER_MINUTES);
            int newInterval = currentInterval + EMERGENCY_UNLOCK_PENALTY;
            preferences.putInt(KEY_INTERVAL_MINUTES, newInterval);
            
            timer.stop();
            transitionToState(UNLOCKED);
            servoControl.unlock();
            
            response["success"] = true;
            response["penalty"] = EMERGENCY_UNLOCK_PENALTY;
            response["message"] = "Emergency unlock granted";
            
            Serial.println("🚨 Emergency unlock via web interface");
        } else {
            response["success"] = false;
            response["message"] = "Emergency unlock limit reached";
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        request->send(200, "application/json", responseStr);
    });
    
    // API endpoint: Reset progress
    server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Reset all stored data
        preferences.clear();
        
        // Reset current state
        timer.stop();
        transitionToState(UNLOCKED);
        servoControl.unlock();
        
        DynamicJsonDocument response(256);
        response["success"] = true;
        response["message"] = "Progress reset";
        
        String responseStr;
        serializeJson(response, responseStr);
        request->send(200, "application/json", responseStr);
        
        Serial.println("🔄 Progress reset via web interface");
    });
    
    // API endpoint: Test servo
    server.on("/api/test", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println("🧪 Testing servo...");
        
        // Test servo movement
        servoControl.unlock();
        delay(1000);
        servoControl.lock();
        delay(1000);
        
        // Return to current state
        if (currentState == UNLOCKED) {
            servoControl.unlock();
        } else {
            servoControl.lock();
        }
        
        DynamicJsonDocument response(256);
        response["success"] = true;
        response["message"] = "Servo test completed";
        
        String responseStr;
        serializeJson(response, responseStr);
        request->send(200, "application/json", responseStr);
    });
    
    // API endpoint: Schedule information
    server.on("/api/schedule-info", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        
        TimerMode currentMode = (TimerMode)preferences.getInt(KEY_TIMER_MODE, FIXED_INTERVAL);
        
        if (currentMode == DAILY_SCHEDULE || currentMode == WEEKLY_SCHEDULE) {
            doc["nextUnlock"] = timer.getNextUnlockTime();
            doc["timeUntilUnlock"] = timer.getTimeUntilNextScheduledUnlock();
        } else {
            doc["nextUnlock"] = "Not scheduled";
            doc["timeUntilUnlock"] = 0;
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // AI Configuration endpoints
    server.on("/api/ai/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(1024);
        
        doc["enabled"] = preferences.getBool("ai_enabled", false);
        doc["provider"] = preferences.getString("ai_provider", "simple");
        doc["apiKey"] = preferences.getString("ai_api_key", "");
        doc["delayMinutes"] = preferences.getInt("ai_delay_min", 10);
        doc["personality"] = preferences.getString("ai_personality", "supportive");
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/ai/config", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Handle AI configuration updates
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, (char*)data);
        
        preferences.putBool("ai_enabled", doc["enabled"]);
        preferences.putString("ai_provider", doc["provider"].as<String>());
        preferences.putString("ai_api_key", doc["apiKey"].as<String>());
        preferences.putInt("ai_delay_min", doc["delayMinutes"]);
        preferences.putString("ai_personality", doc["personality"].as<String>());
        
        DynamicJsonDocument response(256);
        response["success"] = true;
        response["message"] = "AI configuration saved";
        
        String responseStr;
        serializeJson(response, responseStr);
        request->send(200, "application/json", responseStr);
    });

    // AI Emergency unlock endpoint
    server.on("/api/emergency/ai", HTTP_POST, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument response(512);
        
        bool aiEnabled = preferences.getBool("ai_enabled", false);
        if (!aiEnabled) {
            response["success"] = false;
            response["message"] = "AI Emergency Gatekeeper not enabled";
            String responseStr;
            serializeJson(response, responseStr);
            request->send(400, "application/json", responseStr);
            return;
        }

        // Check network restrictions
        if (!isEmergencyAllowedOnCurrentNetwork()) {
            response["success"] = false;
            response["message"] = "Emergency unlock blocked on this network";
            String responseStr;
            serializeJson(response, responseStr);
            request->send(403, "application/json", responseStr);
            return;
        }

        // Start AI session
        String trigger = request->getParam("trigger", true) ? request->getParam("trigger", true)->value() : "general";
        startEmergencySession(trigger);
        
        response["success"] = true;
        response["aiSession"] = true;
        response["sessionId"] = currentEmergencySession.sessionId;
        response["minDuration"] = AI_EMERGENCY_DELAY_MINUTES * 60; // seconds
        response["message"] = "AI Emergency session started";
        
        String responseStr;
        serializeJson(response, responseStr);
        request->send(200, "application/json", responseStr);
    });

    // AI Chat endpoint
    server.on("/api/ai/chat", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Handle AI chat messages
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        DynamicJsonDocument doc(512);
        deserializeJson(doc, (char*)data);
        
        DynamicJsonDocument response(1024);
        
        if (!currentEmergencySession.active) {
            response["success"] = false;
            response["message"] = "No active emergency session";
            String responseStr;
            serializeJson(response, responseStr);
            request->send(400, "application/json", responseStr);
            return;
        }

        String userMessage = doc["message"];
        String personality = preferences.getString("ai_personality", "supportive");
        
        String aiResponse = getAIResponse(userMessage, currentEmergencySession.trigger, personality);
        currentEmergencySession.messageCount++;
        
        unsigned long elapsed = (millis() - currentEmergencySession.startTime) / 1000;
        unsigned long required = AI_EMERGENCY_DELAY_MINUTES * 60;
        bool canUnlock = elapsed >= required && currentEmergencySession.messageCount >= 5;
        
        response["success"] = true;
        response["message"] = aiResponse;
        response["elapsed"] = elapsed;
        response["required"] = required;
        response["canUnlock"] = canUnlock;
        response["messageCount"] = currentEmergencySession.messageCount;
        
        String responseStr;
        serializeJson(response, responseStr);
        request->send(200, "application/json", responseStr);
    });

    // Complete AI emergency unlock
    server.on("/api/emergency/ai/complete", HTTP_POST, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument response(256);
        
        if (!currentEmergencySession.active) {
            response["success"] = false;
            response["message"] = "No active emergency session";
            String responseStr;
            serializeJson(response, responseStr);
            request->send(400, "application/json", responseStr);
            return;
        }

        unsigned long elapsed = (millis() - currentEmergencySession.startTime) / 1000;
        unsigned long required = AI_EMERGENCY_DELAY_MINUTES * 60;
        
        if (elapsed >= required && currentEmergencySession.messageCount >= 5) {
            // Grant emergency unlock
            int emergencyCount = preferences.getInt(KEY_EMERGENCY_COUNT, 0);
            preferences.putInt(KEY_EMERGENCY_COUNT, emergencyCount + 1);
            
            // Add penalty
            int currentInterval = preferences.getInt(KEY_INTERVAL_MINUTES, DEFAULT_TIMER_MINUTES);
            int newInterval = currentInterval + (EMERGENCY_UNLOCK_PENALTY * 2); // Double penalty for AI bypass
            preferences.putInt(KEY_INTERVAL_MINUTES, newInterval);
            
            timer.stop();
            transitionToState(UNLOCKED);
            servoControl.unlock();
            
            // End session
            currentEmergencySession.active = false;
            
            response["success"] = true;
            response["penalty"] = EMERGENCY_UNLOCK_PENALTY * 2;
            response["message"] = "Emergency unlock granted after AI session";
            
            Serial.println("🤖 AI Emergency unlock granted");
        } else {
            response["success"] = false;
            response["message"] = "Session requirements not met";
        }
        
        String responseStr;
        serializeJson(response, responseStr);
        request->send(200, "application/json", responseStr);
    });

    // Security configuration (alias for network config to match frontend expectations)
    server.on("/api/security/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(1024);
        
        doc["allowedNetworks"] = preferences.getString("allowed_networks", "[]");
        doc["blockedNetworks"] = preferences.getString("blocked_networks", "[]");
        doc["blockOnPublic"] = preferences.getBool("block_on_public", false);
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/security/config", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Handle security configuration
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, (char*)data);
        
        preferences.putString("allowed_networks", doc["allowedNetworks"].as<String>());
        preferences.putString("blocked_networks", doc["blockedNetworks"].as<String>());
        preferences.putBool("block_on_public", doc["blockOnPublic"]);
        
        DynamicJsonDocument response(256);
        response["success"] = true;
        response["message"] = "Security configuration saved";
        
        String responseStr;
        serializeJson(response, responseStr);
        request->send(200, "application/json", responseStr);
    });
    
    // Language and cost configuration endpoints
    server.on("/api/language", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        doc["currentLanguage"] = languageConfig.currentLanguage;
        doc["supportedLanguages"] = languageConfig.supportedLanguages;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    server.on("/api/language", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("language", true)) {
            String language = request->getParam("language", true)->value();
            
            // Validate language is supported
            if (languageConfig.supportedLanguages.indexOf(language) >= 0) {
                languageConfig.currentLanguage = language;
                preferences.putString("current_language", language);
                
                DynamicJsonDocument doc(256);
                doc["success"] = true;
                doc["language"] = language;
                
                String response;
                serializeJson(doc, response);
                request->send(200, "application/json", response);
            } else {
                request->send(400, "application/json", "{\"error\":\"Unsupported language\"}");
            }
        } else {
            request->send(400, "application/json", "{\"error\":\"Language parameter required\"}");
        }
    });
    
    server.on("/api/cost-config", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        doc["productName"] = costConfig.productName;
        doc["currency"] = costConfig.currency;
        doc["usePackPrice"] = costConfig.usePackPrice;
        doc["cigaretteCost"] = costConfig.cigaretteCost;
        doc["packCost"] = costConfig.packCost;
        doc["cigarettesPerPack"] = costConfig.cigarettesPerPack;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    server.on("/api/cost-config", HTTP_POST, [](AsyncWebServerRequest *request) {
        bool updated = false;
        
        if (request->hasParam("productName", true)) {
            costConfig.productName = request->getParam("productName", true)->value();
            preferences.putString("product_name", costConfig.productName);
            updated = true;
        }
        
        if (request->hasParam("currency", true)) {
            costConfig.currency = request->getParam("currency", true)->value();
            preferences.putString("currency", costConfig.currency);
            updated = true;
        }
        
        if (request->hasParam("usePackPrice", true)) {
            costConfig.usePackPrice = request->getParam("usePackPrice", true)->value() == "true";
            preferences.putBool("use_pack_price", costConfig.usePackPrice);
            updated = true;
        }
        
        if (request->hasParam("cigaretteCost", true)) {
            costConfig.cigaretteCost = request->getParam("cigaretteCost", true)->value().toFloat();
            preferences.putFloat("cigarette_cost", costConfig.cigaretteCost);
            updated = true;
        }
        
        if (request->hasParam("packCost", true)) {
            costConfig.packCost = request->getParam("packCost", true)->value().toFloat();
            preferences.putFloat("pack_cost", costConfig.packCost);
            updated = true;
        }
        
        if (request->hasParam("cigarettesPerPack", true)) {
            costConfig.cigarettesPerPack = request->getParam("cigarettesPerPack", true)->value().toInt();
            preferences.putInt("cigarettes_per_pack", costConfig.cigarettesPerPack);
            updated = true;
        }
        
        DynamicJsonDocument doc(256);
        doc["success"] = updated;
        if (!updated) {
            doc["error"] = "No valid parameters provided";
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Developer tools endpoints
    server.on("/api/servo/calibration", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(256);
        doc["locked"] = SERVO_LOCKED_POSITION;
        doc["unlocked"] = SERVO_UNLOCKED_POSITION;
        doc["current"] = servoControl.getCurrentPosition();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    server.on("/api/servo/calibration", HTTP_POST, [](AsyncWebServerRequest *request) {
        bool updated = false;
        int lockedPos = SERVO_LOCKED_POSITION;
        int unlockedPos = SERVO_UNLOCKED_POSITION;
        
        if (request->hasParam("locked", true)) {
            lockedPos = request->getParam("locked", true)->value().toInt();
            if (lockedPos >= 0 && lockedPos <= 180) {
                preferences.putInt("servo_locked_pos", lockedPos);
                updated = true;
            }
        }
        
        if (request->hasParam("unlocked", true)) {
            unlockedPos = request->getParam("unlocked", true)->value().toInt();
            if (unlockedPos >= 0 && unlockedPos <= 180) {
                preferences.putInt("servo_unlocked_pos", unlockedPos);
                updated = true;
            }
        }
        
        DynamicJsonDocument doc(256);
        doc["success"] = updated;
        if (updated) {
            doc["locked"] = lockedPos;
            doc["unlocked"] = unlockedPos;
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    server.on("/api/servo/command", HTTP_POST, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(256);
        
        if (request->hasParam("command", true)) {
            String command = request->getParam("command", true)->value();
            
            if (command == "lock") {
                servoControl.lock();
                doc["success"] = true;
                doc["command"] = "lock";
                doc["position"] = SERVO_LOCKED_POSITION;
            } else if (command == "unlock") {
                servoControl.unlock();
                doc["success"] = true;
                doc["command"] = "unlock";
                doc["position"] = SERVO_UNLOCKED_POSITION;
            } else if (command == "moveTo" && request->hasParam("value", true)) {
                int position = request->getParam("value", true)->value().toInt();
                if (position >= 0 && position <= 180) {
                    servoControl.moveTo(position);
                    doc["success"] = true;
                    doc["command"] = "moveTo";
                    doc["position"] = position;
                } else {
                    doc["success"] = false;
                    doc["error"] = "Invalid position (0-180)";
                }
            } else if (command == "sweep") {
                // Perform servo sweep test
                for (int i = 0; i <= 180; i += 30) {
                    servoControl.moveTo(i);
                    delay(500);
                }
                servoControl.moveTo(90); // Return to center
                doc["success"] = true;
                doc["command"] = "sweep";
                doc["position"] = 90;
            } else if (command == "setLocked" && request->hasParam("value", true)) {
                int position = request->getParam("value", true)->value().toInt();
                if (position >= 0 && position <= 180) {
                    preferences.putInt("servo_locked_pos", position);
                    doc["success"] = true;
                    doc["command"] = "setLocked";
                    doc["position"] = position;
                } else {
                    doc["success"] = false;
                    doc["error"] = "Invalid position (0-180)";
                }
            } else if (command == "setUnlocked" && request->hasParam("value", true)) {
                int position = request->getParam("value", true)->value().toInt();
                if (position >= 0 && position <= 180) {
                    preferences.putInt("servo_unlocked_pos", position);
                    doc["success"] = true;
                    doc["command"] = "setUnlocked";
                    doc["position"] = position;
                } else {
                    doc["success"] = false;
                    doc["error"] = "Invalid position (0-180)";
                }
            } else {
                doc["success"] = false;
                doc["error"] = "Unknown command";
            }
        } else {
            doc["success"] = false;
            doc["error"] = "Command parameter required";
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    server.on("/api/dev/system-info", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        doc["firmware"] = "v1.0.0";
        doc["hardware"] = "ESP32-S3";
        doc["flashSize"] = ESP.getFlashChipSize();
        doc["freeMemory"] = ESP.getFreeHeap();
        doc["cpuFreq"] = ESP.getCpuFreqMHz();
        doc["chipId"] = (uint32_t)ESP.getEfuseMac();
        doc["buildDate"] = __DATE__ " " __TIME__;
        doc["uptime"] = millis();
        doc["wifiSignal"] = WiFi.RSSI();
        
        if (WiFi.status() == WL_CONNECTED) {
            JsonObject network = doc.createNestedObject("networkInfo");
            network["ip"] = WiFi.localIP().toString();
            network["mac"] = WiFi.macAddress();
            network["gateway"] = WiFi.gatewayIP().toString();
            network["dns"] = WiFi.dnsIP().toString();
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Serve dev.html only if specifically requested
    server.on("/dev", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/dev.html", "text/html");
    });
    
    // WiFi Management API endpoints
    server.on("/api/wifi/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        doc["connected"] = wifiConnected;
        doc["ip"] = wifiConnected ? WiFi.localIP().toString() : "";
        doc["ssid"] = wifiConnected ? WiFi.SSID() : "";
        doc["rssi"] = wifiConnected ? WiFi.RSSI() : 0;
        doc["apMode"] = WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA;
        doc["apSSID"] = AP_SSID;
        doc["apIP"] = WiFi.softAPIP().toString();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    server.on("/api/wifi/connect", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        // Handle JSON body data
        String body = String((char*)data);
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            // Fallback to form parameters for compatibility
            if (!request->hasParam("ssid", true)) {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"SSID required\"}");
                return;
            }
            
            String ssid = request->getParam("ssid", true)->value();
            String password = request->hasParam("password", true) ? request->getParam("password", true)->value() : "";
            
            if (ssid.length() > 0) {
                // Store credentials
                preferences.putString("wifi_ssid", ssid);
                preferences.putString("wifi_password", password);
                
                // Attempt connection
                WiFi.mode(WIFI_STA);
                WiFi.begin(ssid.c_str(), password.c_str());
                
                Serial.printf("🔄 Attempting WiFi connection to: %s\n", ssid.c_str());
                
                request->send(200, "application/json", "{\"success\":true,\"message\":\"Connection initiated\"}");
            } else {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid SSID\"}");
            }
            return;
        }
        
        // Parse JSON data
        String ssid = doc["ssid"] | "";
        String password = doc["password"] | "";
        
        if (ssid.length() > 0) {
            // Store credentials
            preferences.putString("wifi_ssid", ssid);
            preferences.putString("wifi_password", password);
            
            // Attempt connection
            WiFi.mode(WIFI_STA);
            WiFi.begin(ssid.c_str(), password.c_str());
            
            Serial.printf("🔄 Attempting WiFi connection to: %s\n", ssid.c_str());
            
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Connection initiated\"}");
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"SSID required\"}");
        }
    });
    
    server.on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
        int n = WiFi.scanNetworks();
        DynamicJsonDocument doc(2048);
        JsonArray networks = doc.createNestedArray("networks");
        
        for (int i = 0; i < n; i++) {
            JsonObject network = networks.createNestedObject();
            network["ssid"] = WiFi.SSID(i);
            network["rssi"] = WiFi.RSSI(i);
            network["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Setup check endpoint - determines if box needs initial configuration
    server.on("/api/setup-status", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        
        // Check if basic configuration is complete
        bool hasWifiConfig = preferences.getString("wifi_ssid", "").length() > 0;
        bool hasTimerConfig = preferences.getInt("timer_mode", -1) >= 0;
        bool hasServoCalibration = preferences.getInt("servo_locked_pos", -1) >= 0 && 
                                 preferences.getInt("servo_unlocked_pos", -1) >= 0;
        bool hasCostConfig = preferences.getString("product_name", "").length() > 0;
        
        bool isConfigured = hasTimerConfig && hasServoCalibration;
        
        doc["configured"] = isConfigured;
        doc["firstRun"] = !isConfigured;
        doc["checks"]["wifi"] = hasWifiConfig;
        doc["checks"]["timer"] = hasTimerConfig;
        doc["checks"]["servo"] = hasServoCalibration;
        doc["checks"]["cost"] = hasCostConfig;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Handle 404
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "File not found");
    });
    
    server.begin();
    Serial.println("✅ Web server started");
}

void updateDisplay() {
    TimerMode currentMode = (TimerMode)preferences.getInt(KEY_TIMER_MODE, FIXED_INTERVAL);
    
    switch (currentState) {
        case LOCKED:
            if (currentMode == DAILY_SCHEDULE || currentMode == WEEKLY_SCHEDULE) {
                // Show time until next scheduled unlock
                unsigned long timeUntilUnlock = timer.getTimeUntilNextScheduledUnlock();
                display.showCountdown(timeUntilUnlock);
            } else {
                // Show regular timer countdown
                display.showCountdown(timer.getTimeRemaining() / 1000);
            }
            break;
        case UNLOCKED:
            display.showUnlocked();
            break;
        case COUNTDOWN:
            display.showCountdown(timer.getTimeRemaining() / 1000);
            break;
        case SETUP:
            display.showSetup(wifiConnected);
            break;
        default:
            display.showStatus("Unknown State");
            break;
    }
}

void transitionToState(BoxState newState) {
    if (currentState != newState) {
        Serial.printf("🔄 State transition: %d -> %d\n", currentState, newState);
        currentState = newState;
        
        // Trigger immediate display update
        lastDisplayUpdate = 0;
    }
}

void saveStatus() {
    preferences.putInt("current_state", currentState);
    preferences.putULong64("last_save", millis());
}

void loadConfiguration() {
    currentMode = (TimerMode)preferences.getInt(KEY_TIMER_MODE, FIXED_INTERVAL);
    
    // Load language configuration
    languageConfig.currentLanguage = preferences.getString("current_language", "en");
    languageConfig.supportedLanguages = preferences.getString("supported_languages", "en,pt,es,fr,de");
    
    // Load cost configuration
    costConfig.productName = preferences.getString("product_name", "Cigarettes");
    costConfig.currency = preferences.getString("currency", "EUR");
    costConfig.usePackPrice = preferences.getBool("use_pack_price", false);
    costConfig.cigaretteCost = preferences.getFloat("cigarette_cost", 0.50);
    costConfig.packCost = preferences.getFloat("pack_cost", 10.00);
    costConfig.cigarettesPerPack = preferences.getInt("cigarettes_per_pack", 20);
    
    // Load schedule configuration for scheduled modes
    if (currentMode == DAILY_SCHEDULE || currentMode == WEEKLY_SCHEDULE) {
        int hour = preferences.getInt(KEY_DAILY_HOUR, 22);
        int minute = preferences.getInt(KEY_DAILY_MINUTE, 0);
        int unlockDuration = preferences.getInt(KEY_UNLOCK_DURATION, 30);
        
        if (currentMode == DAILY_SCHEDULE) {
            timer.setDailySchedule(hour, minute, unlockDuration);
        } else if (currentMode == WEEKLY_SCHEDULE) {
            int weekDay = preferences.getInt(KEY_WEEKLY_DAY, 0);
            timer.setWeeklySchedule(weekDay, hour, minute, unlockDuration);
        }
        
        Serial.printf("📅 Loaded schedule: %02d:%02d for %d minutes\n", hour, minute, unlockDuration);
    }
    
    Serial.printf("📖 Loaded configuration - Mode: %d, Language: %s, Currency: %s\n", 
                  currentMode, languageConfig.currentLanguage.c_str(), costConfig.currency.c_str());
}

String getStatusJSON() {
    DynamicJsonDocument doc(1024);
    
    TimerMode currentMode = (TimerMode)preferences.getInt(KEY_TIMER_MODE, FIXED_INTERVAL);
    
    doc["boxState"] = currentState;
    doc["timerMode"] = currentMode;
    doc["timerActive"] = timer.isActive();
    doc["emergencyCount"] = preferences.getInt(KEY_EMERGENCY_COUNT, 0);
    doc["maxEmergency"] = MAX_EMERGENCY_UNLOCKS_PER_DAY;
    
    // Enhanced statistics
    doc["totalCigarettes"] = preferences.getInt(KEY_TOTAL_CIGARETTES, 0);
    doc["smokeFree"] = preferences.getULong64("smoke_free_days", 0);
    doc["moneySaved"] = preferences.getFloat("money_saved", 0.0);
    doc["totalDays"] = preferences.getULong64("total_days", 0);
    doc["longestStreak"] = preferences.getInt("longest_streak", 0);
    doc["wifiConnected"] = wifiConnected;
    doc["currentNetwork"] = WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "AP Mode";
    
    // AI Emergency Gatekeeper status
    doc["aiEnabled"] = preferences.getBool("ai_enabled", false);
    doc["emergencyAllowed"] = isEmergencyAllowedOnCurrentNetwork();
    doc["activeSession"] = currentEmergencySession.active;
    
    if (currentEmergencySession.active) {
        unsigned long elapsed = (millis() - currentEmergencySession.startTime) / 1000;
        doc["sessionElapsed"] = elapsed;
        doc["sessionRequired"] = AI_EMERGENCY_DELAY_MINUTES * 60;
        doc["messageCount"] = currentEmergencySession.messageCount;
    }
    
    // Add time information based on mode
    if (currentMode == DAILY_SCHEDULE || currentMode == WEEKLY_SCHEDULE) {
        doc["timeRemaining"] = timer.getTimeUntilNextScheduledUnlock();
        doc["nextUnlock"] = timer.getNextUnlockTime();
        doc["isScheduled"] = true;
    } else {
        doc["timeRemaining"] = timer.getTimeRemaining() / 1000; // Convert to seconds
        doc["isScheduled"] = false;
    }
    
    String response;
    serializeJson(doc, response);
    return response;
}

// AI Emergency Gatekeeper functions
bool isEmergencyAllowedOnCurrentNetwork() {
    if (WiFi.status() != WL_CONNECTED) {
        return true; // Allow on AP mode
    }
    
    String currentSSID = WiFi.SSID();
    bool blockOnPublic = preferences.getBool("block_on_public", false);
    
    // Check if network appears to be public
    if (blockOnPublic && (currentSSID.indexOf("Free") >= 0 || 
                         currentSSID.indexOf("Public") >= 0 || 
                         currentSSID.indexOf("Guest") >= 0 ||
                         currentSSID.indexOf("WiFi") >= 0)) {
        Serial.printf("🚫 Emergency blocked on public network: %s\n", currentSSID.c_str());
        return false;
    }
    
    // Parse allowed networks JSON
    String allowedNetworksJson = preferences.getString("allowed_networks", "[]");
    DynamicJsonDocument allowedDoc(512);
    deserializeJson(allowedDoc, allowedNetworksJson);
    
    if (allowedDoc.size() > 0) {
        bool found = false;
        for (size_t i = 0; i < allowedDoc.size(); i++) {
            if (allowedDoc[i].as<String>() == currentSSID) {
                found = true;
                break;
            }
        }
        if (!found) {
            Serial.printf("🚫 Emergency blocked - network not in allowed list: %s\n", currentSSID.c_str());
            return false;
        }
    }
    
    // Parse blocked networks JSON
    String blockedNetworksJson = preferences.getString("blocked_networks", "[]");
    DynamicJsonDocument blockedDoc(512);
    deserializeJson(blockedDoc, blockedNetworksJson);
    
    for (size_t i = 0; i < blockedDoc.size(); i++) {
        if (blockedDoc[i].as<String>() == currentSSID) {
            Serial.printf("🚫 Emergency blocked on blocked network: %s\n", currentSSID.c_str());
            return false;
        }
    }
    
    return true; // Allow by default
}

void startEmergencySession(String trigger) {
    currentEmergencySession.active = true;
    currentEmergencySession.startTime = millis();
    currentEmergencySession.sessionId = String(millis(), HEX);
    currentEmergencySession.messageCount = 0;
    currentEmergencySession.trigger = trigger;
    
    // Save session ID to preferences (for tracking)
    preferences.putString("current_session_id", currentEmergencySession.sessionId);
    
    Serial.printf("🚀 Emergency session started: %s (Trigger: %s)\n", currentEmergencySession.sessionId.c_str(), trigger.c_str());
}

String getAIResponse(String userMessage, String trigger, String personality) {
    String provider = preferences.getString("ai_provider", "simple");
    
    if (provider == "openai") {
        return getOpenAIResponse(userMessage, trigger, personality);
    } else if (provider == "local") {
        return getLocalAIResponse(userMessage, trigger, personality);
    } else {
        // Simple rule-based responses
        return getSimpleAIResponse(userMessage, trigger, personality);
    }
}

String getSimpleAIResponse(String userMessage, String trigger, String personality) {
    return getEnhancedAIResponse(userMessage, trigger, personality, currentEmergencySession.messageCount);
}

String getEnhancedAIResponse(String userMessage, String trigger, String personality, int messageCount) {
    String response = "";
    
    // Progressive conversation flow
    if (messageCount == 1) {
        response = getWelcomeMessage(personality);
    } else if (messageCount == 2) {
        response = getTriggerAnalysis(userMessage, trigger, personality);
    } else if (messageCount == 3) {
        response = getReflectionQuestion(trigger, 0);
    } else if (messageCount == 4) {
        response = analyzeReflectionResponse(userMessage, trigger, personality);
    } else if (messageCount == 5) {
        response = getCopingStrategy(trigger, personality);
    } else if (messageCount >= 6) {
        response = getEncouragementAndAlternatives(userMessage, trigger, personality);
    }
    
    return response;
}

String getWelcomeMessage(String personality) {
    if (personality == "supportive") {
        return "Hi there. I can sense you're having a tough moment right now, and I want you to know that reaching out shows incredible strength. Let's work through this together. What's going on that's making you want to smoke right now?";
    } else if (personality == "strict") {
        return "Hold on. Before we go any further, I need you to remember why you started this journey. You made a commitment to yourself for a reason. What was that reason, and why are you willing to throw it away right now?";
    } else if (personality == "understanding") {
        return "Hey, I'm here with you. No judgment, no lectures - just someone who gets that this is really hard. Take a deep breath and tell me what's happening in your world right now that's making this craving feel so intense.";
    } else if (personality == "professional") {
        return "Good evening. What you're experiencing right now is a completely normal part of the neuroplasticity process during smoking cessation. Let's examine the triggers and develop a cognitive strategy. Can you describe the specific circumstances that led to this craving?";
    }
    return "I'm here to help you through this moment. What's triggering this craving?";
}

String getTriggerAnalysis(String userMessage, String trigger, String personality) {
    String response = "";
    
    if (personality == "supportive") {
        response = "Thank you for sharing that with me. I can hear in your words that this is genuinely difficult. ";
        if (trigger == "stress") {
            response += "Stress can make everything feel overwhelming, but smoking won't actually solve what's stressing you - it just adds another layer of complexity. ";
        } else if (trigger == "boredom") {
            response += "Boredom can be surprisingly challenging because our minds start seeking familiar patterns, and smoking has been one of those patterns. ";
        } else if (trigger == "anger") {
            response += "Anger is such a powerful emotion, and it makes sense that you'd want to do something with that energy. ";
        } else if (trigger == "habit") {
            response += "Habits are deeply ingrained patterns, and it takes real awareness to even notice when they're activated. ";
        }
    } else if (personality == "strict") {
        response = "I hear what you're saying, but let me challenge you on something. ";
        if (trigger == "stress") {
            response += "Stress is a part of life - successful people learn to handle it without crutches. When you smoke during stress, you're teaching your brain that you can't handle life's challenges. ";
        } else if (trigger == "boredom") {
            response += "Boredom is a choice. There are literally thousands of things you could do right now instead of smoking. The fact that you're choosing the destructive option tells me something about your priorities. ";
        } else if (trigger == "anger") {
            response += "Anger is energy. You can use that energy to destroy your progress, or you can channel it into something that actually solves the problem. ";
        } else if (trigger == "habit") {
            response += "Habits are just repeated choices. Every time you choose to smoke, you're choosing to stay trapped in the same pattern. ";
        }
    } else if (personality == "understanding") {
        response = "I really appreciate you being honest with me about what's going on. ";
        if (trigger == "stress") {
            response += "Stress is one of the hardest triggers because it feels so immediate and overwhelming. Your brain is looking for the fastest relief it knows. ";
        } else if (trigger == "boredom") {
            response += "Boredom hits different when you're trying to quit because suddenly you have all this time and mental space that used to be filled with smoking. ";
        } else if (trigger == "anger") {
            response += "Anger can feel like it needs an immediate outlet, and smoking has probably been that outlet for a while. ";
        } else if (trigger == "habit") {
            response += "Habitual cravings are tricky because they can hit you even when you don't really want to smoke - it's just what your brain expects to do. ";
        }
    } else if (personality == "professional") {
        response = "Based on your description, we can identify specific neural pathways being activated. ";
        if (trigger == "stress") {
            response += "Stress activates the hypothalamic-pituitary-adrenal axis, which has been conditioned to expect nicotine as a coping mechanism. ";
        } else if (trigger == "boredom") {
            response += "Dopamine-seeking behavior often manifests during understimulation, when the brain seeks familiar reward patterns. ";
        } else if (trigger == "anger") {
            response += "Emotional dysregulation often triggers conditioned responses that were developed as self-soothing mechanisms. ";
        } else if (trigger == "habit") {
            response += "Automatic behavioral patterns stored in the basal ganglia are being activated by environmental or temporal cues. ";
        }
    }
    
    return response + "Let me ask you something that might help us understand this better...";
}

String getReflectionQuestion(String trigger, int questionNumber) {
    String questions[5];
    
    if (trigger == "stress") {
        questions[0] = "If you could solve the stress you're feeling right now without smoking, what would that solution look like?";
        questions[1] = "Think back to a time when you handled stress really well without smoking. What was different about that situation?";
        questions[2] = "What's the worst thing that would happen if you just sat with this stress for 10 more minutes without doing anything?";
        questions[3] = "If your best friend was feeling this exact stress, what advice would you give them?";
        questions[4] = "How will you feel about smoking in exactly one hour from now?";
    } else if (trigger == "boredom") {
        questions[0] = "What's something you've been putting off that you could tackle right now instead?";
        questions[1] = "If you had to choose between being bored for 10 minutes or disappointed in yourself for hours, which would you pick?";
        questions[2] = "What activities used to excite you before smoking became your go-to boredom fix?";
        questions[3] = "If someone gave you $50 to stay busy for the next hour without smoking, what would you do?";
        questions[4] = "What would happen if you just let yourself be bored for a few minutes without trying to fix it?";
    } else if (trigger == "anger") {
        questions[0] = "What are you really angry about - the situation, or feeling like you need a cigarette to handle it?";
        questions[1] = "If you smoke right now, will you be less angry, or just angry AND disappointed in yourself?";
        questions[2] = "What would handling this anger like a complete badass look like?";
        questions[3] = "What's the angriest you've ever been when you handled it perfectly without smoking?";
        questions[4] = "Will the thing you're angry about matter in a week? Will breaking your quit attempt matter in a week?";
    } else if (trigger == "habit") {
        questions[0] = "What usually happens right before this habitual moment that we could change?";
        questions[1] = "If you had to replace this smoking habit with a different 5-minute habit, what would it be?";
        questions[2] = "What's the real reward your brain is seeking right now - the nicotine, or the break/pause/ritual?";
        questions[3] = "How could you give yourself the same mental break without the cigarette?";
        questions[4] = "What would your future self, who successfully quit, tell you about this moment?";
    } else {
        questions[0] = "What specific situation or feeling triggered this craving right now?";
        questions[1] = "How has smoking helped you in similar situations before, and did it actually solve the underlying issue?";
        questions[2] = "What would you do right now if smoking wasn't an option at all?";
        questions[3] = "What's one thing you could do in the next 5 minutes that would make you feel proud of yourself?";
        questions[4] = "If you successfully resist this craving, how will you feel about yourself tomorrow?";
    }
    
    return questions[questionNumber % 5];
}

String analyzeReflectionResponse(String userMessage, String trigger, String personality) {
    String response = "";
    
    if (personality == "supportive") {
        response = "That's a really thoughtful answer. I can see you're genuinely thinking about this, which shows you care about your success. ";
        response += "What you just shared actually gives us a path forward. Instead of smoking, what if we tried to address what you identified? ";
    } else if (personality == "strict") {
        response = "Good. Now you're thinking instead of just reacting. That's exactly the mental discipline you need to succeed. ";
        response += "You have the answer right there - you just need to act on it instead of taking the easy way out. ";
    } else if (personality == "understanding") {
        response = "Thank you for really considering that question. Your answer tells me you have more insight into this than you might realize. ";
        response += "It sounds like there might be a way to handle this situation that honors both your feelings and your commitment to quitting. ";
    } else if (personality == "professional") {
        response = "Your response indicates good self-awareness and cognitive flexibility. This suggests your prefrontal cortex is engaging effectively. ";
        response += "Let's build on this insight with a specific behavioral intervention strategy. ";
    }
    
    return response + "Here's what I'd like you to try instead...";
}

String getCopingStrategy(String trigger, String personality) {
    String strategy = "";
    
    if (trigger == "stress") {
        if (personality == "supportive") {
            strategy = "Let's do a quick stress-release technique together. First, take 3 deep breaths - in for 4 counts, hold for 4, out for 6. Then, write down or mentally list 3 things that are within your control right now, even if they're small. Focus on just one of those things for the next 10 minutes.";
        } else if (personality == "strict") {
            strategy = "Stop making excuses and start making solutions. Right now: 1) Stand up and do 20 jumping jacks, 2) Write down exactly what's stressing you, 3) Write down one action you can take today to improve it, 4) Take that action. Stress doesn't give you permission to quit your quit.";
        } else if (personality == "understanding") {
            strategy = "It's okay to feel stressed - that's part of being human. Try this: place your hand on your chest and feel your heartbeat. Remind yourself that this stress will pass, just like all the other stresses you've survived. Maybe do something that usually comforts you - listen to music, take a warm shower, or call someone who cares about you.";
        } else if (personality == "professional") {
            strategy = "Implement a structured stress response protocol: 1) Progressive muscle relaxation (tense and release each muscle group for 5 seconds), 2) Cognitive reframing (identify the specific stressor and three potential solutions), 3) Grounding technique (name 5 things you can see, 4 you can touch, 3 you can hear).";
        }
    } else if (trigger == "boredom") {
        if (personality == "supportive") {
            strategy = "Boredom is actually a gift - it means your mind is ready for something new. How about: text someone you haven't talked to in a while, organize one small area of your space, or spend 10 minutes learning something random online. The goal isn't to be productive, just to redirect your brain's energy.";
        } else if (personality == "strict") {
            strategy = "Boredom is a luxury problem. Do 50 push-ups right now, then clean something, then learn something. If you have time to be bored, you have time to improve yourself. Make this craving cost you something positive instead of something destructive.";
        } else if (personality == "understanding") {
            strategy = "Being bored when you're trying to quit can feel really uncomfortable because you're used to filling that space with smoking. It's okay to just sit with the boredom for a minute. Maybe try a gentle activity like stretching, making tea, or just looking out the window and noticing what you see.";
        } else if (personality == "professional") {
            strategy = "Engage your dopamine system through novel, low-commitment activities: 1) Learn three new facts about a topic that interests you, 2) Rearrange your immediate environment, 3) Practice a brief mindfulness exercise focusing on sensory input rather than thought suppression.";
        }
    } else if (trigger == "anger") {
        if (personality == "supportive") {
            strategy = "That anger is real and valid. Let's channel it constructively: try doing vigorous exercise for 2-3 minutes, punch a pillow, or write an angry letter you'll never send. Your anger doesn't need a cigarette to be heard - it needs movement and expression.";
        } else if (personality == "strict") {
            strategy = "Use that anger as fuel. Angry at the situation? Fix it. Angry at yourself? Prove you're stronger. Angry at others? Show them what you're made of by succeeding. Channel that fire into determination, not destruction.";
        } else if (personality == "understanding") {
            strategy = "Anger can feel so intense and urgent. It's completely understandable that you'd want to do something with that energy. Try this: set a timer for 5 minutes and let yourself feel angry without judging it. Sometimes anger just needs to be acknowledged before it can pass.";
        } else if (personality == "professional") {
            strategy = "Implement emotional regulation through physiological intervention: 1) Cold water on wrists and face to activate the dive response, 2) Bilateral stimulation (alternate tapping left and right sides of your body), 3) Expressive writing for 90 seconds to process the emotional content.";
        }
    } else if (trigger == "habit") {
        if (personality == "supportive") {
            strategy = "Habits are so automatic, which is why they're extra tricky. Let's interrupt the pattern: do the first part of your smoking routine, but replace the cigarette with something else - hold a pen like a cigarette, step outside but do jumping jacks instead, or take the break but drink water.";
        } else if (personality == "strict") {
            strategy = "Habits are just weak excuses. You control your actions, not the other way around. Right now: do the opposite of what the habit wants. If you normally smoke sitting down, stand up and pace. If you smoke outside, stay inside. Break the pattern by choosing differently.";
        } else if (personality == "understanding") {
            strategy = "Habitual cravings can sneak up on you because they're not really about wanting to smoke - they're about the familiar routine. Try keeping most of the routine but swapping out the cigarette: take the same break, go to the same place, but chew gum or do breathing exercises instead.";
        } else if (personality == "professional") {
            strategy = "Disrupt the automated behavior chain through pattern interruption: 1) Change your physical position/location, 2) Perform a competing behavior (if you normally use your hands, keep them busy), 3) Introduce a 2-minute delay before any smoking-related action to reactivate conscious decision-making.";
        }
    } else {
        strategy = "Try this immediate action plan: 1) Change your environment (move to a different room or go outside), 2) Do something physical for 2 minutes (walk, stretch, clean), 3) Engage your senses (smell something pleasant, listen to music, taste mint gum), 4) Connect with your motivation (look at a photo that reminds you why you're quitting).";
    }
    
    return strategy;
}

String getEncouragementAndAlternatives(String userMessage, String trigger, String personality) {
    String response = "";
    
    if (personality == "supportive") {
        response = "You've been talking with me for several minutes now, which means you're already winning. Every second you delay is your brain building new, healthier pathways. I'm proud of how you're handling this. ";
        response += "Here are three things you could do right now instead: 1) Take a hot shower, 2) Call someone who makes you laugh, 3) Go for a walk around the block. Which one feels most appealing?";
    } else if (personality == "strict") {
        response = "You've proven you can resist for this long, which means you're stronger than your addiction. Don't waste that strength now. ";
        response += "Your options: 1) Go exercise until you're tired, 2) Do something productive you've been avoiding, 3) Face this craving head-on and prove you're in control. What's it going to be?";
    } else if (personality == "understanding") {
        response = "Look how far you've come in this conversation. That's not accident - that's your real self choosing your long-term happiness over short-term relief. ";
        response += "Some gentle alternatives: 1) Make yourself a special drink (tea, coffee, smoothie), 2) Put on music that makes you feel good, 3) Do something nice for someone else. What sounds good to you?";
    } else if (personality == "professional") {
        response = "You've successfully extended the decision-making window and engaged higher-order cognitive processes. This indicates strong executive function and impulse control capacity. ";
        response += "Evidence-based alternatives include: 1) 4-7-8 breathing technique (proven to reduce cortisol), 2) Progressive muscle relaxation (activates parasympathetic nervous system), 3) Brief high-intensity exercise (releases endorphins naturally). Which intervention appeals to your current psychological state?";
    }
    
    return response;
}

String getOpenAIResponse(String userMessage, String trigger, String personality) {
    String apiKey = preferences.getString("ai_api_key", "");
    if (apiKey.length() == 0) {
        return "OpenAI API key not configured. Please set it in Settings.";
    }
    
    HTTPClient http;
    http.begin("https://api.openai.com/v1/chat/completions");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + apiKey);
    
    // Construct prompt based on personality and trigger
    String systemPrompt = "You are a " + personality + " smoking cessation counselor. ";
    systemPrompt += "The user is experiencing a '" + trigger + "' trigger and wants to access cigarettes. ";
    systemPrompt += "Your goal is to help them resist this urge through conversation, coping strategies, and encouragement. ";
    systemPrompt += "Be empathetic but firm. Provide practical alternatives. Keep responses under 200 words.";
    
    DynamicJsonDocument requestDoc(1024);
    requestDoc["model"] = "gpt-3.5-turbo";
    requestDoc["messages"][0]["role"] = "system";
    requestDoc["messages"][0]["content"] = systemPrompt;
    requestDoc["messages"][1]["role"] = "user";
    requestDoc["messages"][1]["content"] = userMessage;
    requestDoc["max_tokens"] = 200;
    requestDoc["temperature"] = 0.7;
    
    String requestBody;
    serializeJson(requestDoc, requestBody);
    
    int httpResponseCode = http.POST(requestBody);
    
    if (httpResponseCode == 200) {
        String response = http.getString();
        DynamicJsonDocument responseDoc(2048);
        deserializeJson(responseDoc, response);
        
        String aiResponse = responseDoc["choices"][0]["message"]["content"];
        http.end();
        return aiResponse;
    } else {
        http.end();
        return "AI service temporarily unavailable. Try the simple mode instead.";
    }
}

String getLocalAIResponse(String userMessage, String trigger, String personality) {
    // Placeholder for local AI integration (Ollama, etc.)
    return "Local AI not yet implemented. Using simple responses instead.\n\n" + 
           getSimpleAIResponse(userMessage, trigger, personality);
}

// Reflection Question System
void startReflectionSession(String trigger, String personality) {
    currentReflection.trigger = trigger;
    currentReflection.personality = personality;
    currentReflection.currentQuestion = 0;
    currentReflection.startTime = millis();
    currentReflection.completed = false;
    
    for (int i = 0; i < 5; i++) {
        currentReflection.responses[i] = "";
    }
    
    // Also update the emergency session
    currentEmergencySession.reflectionMode = true;
    currentEmergencySession.currentReflectionQuestion = 0;
    currentEmergencySession.reflectionCompleted = false;
}

String getNextReflectionQuestion() {
    if (currentReflection.currentQuestion >= 5) {
        currentReflection.completed = true;
        currentEmergencySession.reflectionCompleted = true;
        return generateReflectionSummary();
    }
    
    String question = getReflectionQuestion(currentReflection.trigger, currentReflection.currentQuestion);
    return question;
}

void recordReflectionResponse(String response) {
    if (currentReflection.currentQuestion < 5) {
        currentReflection.responses[currentReflection.currentQuestion] = response;
        currentEmergencySession.reflectionResponses[currentReflection.currentQuestion] = response;
        currentReflection.currentQuestion++;
        currentEmergencySession.currentReflectionQuestion++;
    }
}

String generateReflectionSummary() {
    String summary = "You've spent " + String((millis() - currentReflection.startTime) / 60000) + " minutes reflecting on this craving. ";
    summary += "Based on your responses, it seems like the core issue is about ";
    
    if (currentReflection.trigger == "stress") {
        summary += "finding better ways to manage pressure without smoking. ";
    } else if (currentReflection.trigger == "boredom") {
        summary += "creating more engaging activities in your daily routine. ";
    } else if (currentReflection.trigger == "anger") {
        summary += "channeling your anger into constructive actions rather than destructive habits. ";
    } else if (currentReflection.trigger == "habit") {
        summary += "building new, healthier automatic responses to replace old patterns. ";
    } else {
        summary += "understanding your triggers and developing personalized coping strategies. ";
    }
    
    summary += "\n\nYou have the insights you need - now it's about choosing to act on them instead of smoking. ";
    summary += "What specific action will you take right now to honor the reflection work you've just done?";
    
    return summary;
}

bool isReflectionSessionActive() {
    return currentEmergencySession.reflectionMode && !currentEmergencySession.reflectionCompleted;
}

void broadcastStatus() {
    if (ws.count() > 0) {
        String statusJSON = getStatusJSON();
        ws.textAll(statusJSON);
    }
}

void updateStatistics() {
    unsigned long currentTime = millis();
    unsigned long firstStart = preferences.getULong64("first_start", currentTime);
    
    if (firstStart == currentTime) {
        preferences.putULong64("first_start", currentTime);
    }
    
    unsigned long daysSinceStart = (currentTime - firstStart) / 86400000UL;
    int totalCigarettes = preferences.getInt(KEY_TOTAL_CIGARETTES, 0);
    float cigaretteCost = preferences.getFloat("cigarette_cost", 0.50); // Default $0.50 per cigarette
    float moneySaved = totalCigarettes * cigaretteCost;
    
    preferences.putULong64("total_days", daysSinceStart);
    preferences.putFloat("money_saved", moneySaved);
    
    // Calculate smoke-free days (days without smoking)
    unsigned long lastUsage = preferences.getULong64(KEY_LAST_UNLOCK, 0);
    if (lastUsage > 0) {
        unsigned long daysSinceLastUsage = (currentTime - lastUsage) / 86400000UL;
        preferences.putULong64("smoke_free_days", daysSinceLastUsage);
    }
}

void updateGradualReduction() {
    int currentInterval = preferences.getInt(KEY_INTERVAL_MINUTES, DEFAULT_TIMER_MINUTES);
    int usageCount = preferences.getInt(KEY_TOTAL_CIGARETTES, 0);
    
    // Increase interval by 5 minutes every 10 uses
    int newInterval = DEFAULT_TIMER_MINUTES + (usageCount / 10) * 5;
    newInterval = min(newInterval, MAX_TIMER_MINUTES);
    
    if (newInterval > currentInterval) {
        preferences.putInt(KEY_INTERVAL_MINUTES, newInterval);
        Serial.printf("📈 Gradual reduction: interval increased to %d minutes\n", newInterval);
    }
}

void updateCompleteQuitMode() {
    int usageCount = preferences.getInt(KEY_TOTAL_CIGARETTES, 0);
    int currentInterval = preferences.getInt(KEY_INTERVAL_MINUTES, DEFAULT_TIMER_MINUTES);
    
    // Progressive lockout: each use doubles the interval (with max)
    int newInterval = DEFAULT_TIMER_MINUTES * pow(2, usageCount / 5); // Double every 5 uses
    newInterval = min(newInterval, MAX_TIMER_MINUTES);
    
    if (newInterval > currentInterval) {
        preferences.putInt(KEY_INTERVAL_MINUTES, newInterval);
        Serial.printf("🎯 Complete quit mode: interval increased to %d minutes\n", newInterval);
    }
}
