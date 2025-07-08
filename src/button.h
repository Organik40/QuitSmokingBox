#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
public:
    Button(int pin);
    void begin();
    bool isPressed();

private:
    int buttonPin;
};

#endif // BUTTON_H