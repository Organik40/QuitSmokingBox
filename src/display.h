#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

class Display {
public:
    Display();
    bool begin();
    void showWelcome();
    void showCountdown(unsigned long secondsRemaining);
    void showUnlocked();
    void showSetup(bool wifiConnected);
    void showStatus(const char* message);
    void showMessage(const char* message, unsigned long duration = 0);
    void clear();
    void update();

private:
    Adafruit_SSD1306 display;
    unsigned long messageEndTime;
    bool showingMessage;
    
    void drawCenteredText(const char* text, int y, int textSize = 1);
    void drawProgressBar(int percentage, int y);
    String formatTime(unsigned long seconds);
};

#endif // DISPLAY_H