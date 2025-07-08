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

// Global objects
AsyncWebServer server(80);
Preferences preferences;
Display display;
ServoControl servoControl;
Timer timer;
Button button;

// Global variables
BoxState currentState = SETUP;
TimerMode currentMode = FIXED_INTERVAL;
unsigned long lastDisplayUpdate = 0;
unsigned long lastStatusSave = 0;
bool wifiConnected = false;

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
    }
    
    // Update display periodically
    if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = millis();
    }
    
    // Save status periodically
    if (millis() - lastStatusSave >= 60000) { // Every minute
        saveStatus();
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
    Serial.println("📶 Setting up WiFi Access Point...");
    
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
    
    Serial.printf("📖 Loaded configuration - Mode: %d\n", currentMode);
}

String getStatusJSON() {
    DynamicJsonDocument doc(1024);
    
    TimerMode currentMode = (TimerMode)preferences.getInt(KEY_TIMER_MODE, FIXED_INTERVAL);
    
    doc["boxState"] = currentState;
    doc["timerMode"] = currentMode;
    doc["timerActive"] = timer.isActive();
    doc["emergencyCount"] = preferences.getInt(KEY_EMERGENCY_COUNT, 0);
    doc["maxEmergency"] = MAX_EMERGENCY_UNLOCKS_PER_DAY;
    doc["todayCount"] = 0; // Implement daily counting logic
    doc["smokeFree"] = preferences.getInt(KEY_DAYS_SMOKE_FREE, 0);
    doc["totalCigarettes"] = preferences.getInt(KEY_TOTAL_CIGARETTES, 0);
    doc["totalSaved"] = preferences.getInt(KEY_TOTAL_CIGARETTES, 0) * 0.50; // Assume $0.50 per cigarette
    doc["totalDays"] = (millis() / 86400000); // Days since first run
    doc["longestStreak"] = preferences.getInt("longest_streak", 0);
    doc["wifiConnected"] = wifiConnected;
    
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
