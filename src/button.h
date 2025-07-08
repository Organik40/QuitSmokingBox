#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
public:
    Button();
    void begin();
    void update();
    bool wasPressed();
    bool isPressed();

private:
    int buttonPin;
    int lastReading;
    int lastState;
    unsigned long lastDebounceTime;
    bool pressed;
};

#endif // BUTTON_H