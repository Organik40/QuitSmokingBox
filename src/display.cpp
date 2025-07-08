#include "display.h"
#include "config.h"
#include <Preferences.h>

Display::Display() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), 
                     messageEndTime(0), showingMessage(false) {
}

bool Display::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        return false;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();
    
    return true;
}

void Display::showWelcome() {
    display.clearDisplay();
    
    // Draw logo
    display.setTextSize(3);
    drawCenteredText("🚭", 10);
    
    display.setTextSize(1);
    drawCenteredText("QUIT SMOKING", 35);
    drawCenteredText("TIMER BOX", 45);
    drawCenteredText("Starting...", 55);
    
    display.display();
}

void Display::showCountdown(unsigned long secondsRemaining) {
    if (showingMessage && millis() < messageEndTime) {
        return; // Don't update if showing a temporary message
    }
    
    showingMessage = false;
    display.clearDisplay();
    
    // Check if we're in a scheduled mode (we need to include this from main)
    extern Preferences preferences;
    
    TimerMode currentMode = (TimerMode)preferences.getInt(KEY_TIMER_MODE, FIXED_INTERVAL);
    
    if (currentMode == DAILY_SCHEDULE || currentMode == WEEKLY_SCHEDULE) {
        // Show scheduled countdown
        display.setTextSize(1);
        if (currentMode == DAILY_SCHEDULE) {
            drawCenteredText("DAILY SCHEDULE", 0);
        } else {
            drawCenteredText("WEEKLY SCHEDULE", 0);
        }
        
        // Show time until next unlock
        display.setTextSize(2);
        
        // Handle long times differently
        if (secondsRemaining > 86400) { // More than 24 hours
            unsigned long days = secondsRemaining / 86400;
            unsigned long hours = (secondsRemaining % 86400) / 3600;
            String dayStr = String(days) + "d " + String(hours) + "h";
            drawCenteredText(dayStr.c_str(), 20);
        } else {
            String timeStr = formatTime(secondsRemaining);
            drawCenteredText(timeStr.c_str(), 20);
        }
        
        display.setTextSize(1);
        drawCenteredText("Until next unlock", 40);
        
        // Show scheduled time
        int hour = preferences.getInt(KEY_DAILY_HOUR, 22);
        int minute = preferences.getInt(KEY_DAILY_MINUTE, 0);
        char scheduleStr[20];
        snprintf(scheduleStr, sizeof(scheduleStr), "Schedule: %02d:%02d", hour, minute);
        drawCenteredText(scheduleStr, 55);
    } else {
        // Regular timer countdown
        display.setTextSize(1);
        drawCenteredText("LOCKED", 0);
        
        // Main countdown
        String timeStr = formatTime(secondsRemaining);
        display.setTextSize(2);
        drawCenteredText(timeStr.c_str(), 20);
        
        // Progress bar
        unsigned long maxTime = 3600; // 1 hour default
        int percentage = 100 - ((secondsRemaining * 100) / maxTime);
        drawProgressBar(percentage, 45);
        
        // Bottom text
        display.setTextSize(1);
        drawCenteredText("Time until unlock", 55);
    }
    
    display.display();
}

void Display::showUnlocked() {
    if (showingMessage && millis() < messageEndTime) {
        return;
    }
    
    showingMessage = false;
    display.clearDisplay();
    
    // Large unlock icon
    display.setTextSize(3);
    drawCenteredText("🔓", 10);
    
    display.setTextSize(2);
    drawCenteredText("UNLOCKED", 35);
    
    display.setTextSize(1);
    drawCenteredText("Box is ready", 55);
    
    display.display();
}

void Display::showSetup(bool wifiConnected) {
    display.clearDisplay();
    
    display.setTextSize(1);
    drawCenteredText("SETUP MODE", 0);
    
    display.setTextSize(1);
    if (wifiConnected) {
        drawCenteredText("WiFi: Connected", 20);
        drawCenteredText("SSID: QuitSmokingBox", 30);
        drawCenteredText("Connect to configure", 45);
    } else {
        drawCenteredText("WiFi: Connecting...", 20);
        drawCenteredText("Please wait...", 35);
    }
    
    display.display();
}

void Display::showStatus(const char* message) {
    display.clearDisplay();
    
    display.setTextSize(1);
    drawCenteredText("STATUS", 0);
    
    display.setTextSize(1);
    drawCenteredText(message, 30);
    
    display.display();
}

void Display::showMessage(const char* message, unsigned long duration) {
    display.clearDisplay();
    
    display.setTextSize(1);
    drawCenteredText("NOTICE", 0);
    
    display.setTextSize(1);
    // Word wrap for long messages
    String msg = String(message);
    int y = 25;
    int start = 0;
    
    while (start < msg.length()) {
        int end = start + 21; // Approximate characters per line
        if (end >= msg.length()) {
            end = msg.length();
        } else {
            // Try to break at a space
            while (end > start && msg.charAt(end) != ' ') {
                end--;
            }
            if (end == start) {
                end = start + 21; // Force break if no space found
            }
        }
        
        String line = msg.substring(start, end);
        drawCenteredText(line.c_str(), y);
        y += 10;
        start = end + 1;
        
        if (y > 55) break; // Don't overflow display
    }
    
    display.display();
    
    if (duration > 0) {
        showingMessage = true;
        messageEndTime = millis() + duration;
    }
}

void Display::clear() {
    display.clearDisplay();
    display.display();
}

void Display::update() {
    // Check if temporary message should be cleared
    if (showingMessage && millis() >= messageEndTime) {
        showingMessage = false;
        // Force a redraw by clearing
        display.clearDisplay();
        display.display();
    }
}

void Display::drawCenteredText(const char* text, int y, int textSize) {
    display.setTextSize(textSize);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    
    int x = (SCREEN_WIDTH - w) / 2;
    display.setCursor(x, y);
    display.print(text);
}

void Display::drawProgressBar(int percentage, int y) {
    int barWidth = 100;
    int barHeight = 6;
    int x = (SCREEN_WIDTH - barWidth) / 2;
    
    // Draw border
    display.drawRect(x, y, barWidth, barHeight, SSD1306_WHITE);
    
    // Draw fill
    int fillWidth = (barWidth - 2) * percentage / 100;
    if (fillWidth > 0) {
        display.fillRect(x + 1, y + 1, fillWidth, barHeight - 2, SSD1306_WHITE);
    }
}

String Display::formatTime(unsigned long seconds) {
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;
    
    char buffer[10];
    if (hours > 0) {
        sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes, secs);
    } else {
        sprintf(buffer, "%02lu:%02lu", minutes, secs);
    }
    
    return String(buffer);
}