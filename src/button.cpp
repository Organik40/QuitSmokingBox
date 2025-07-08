#include "button.h"
#include "config.h"

Button::Button() {
    buttonPin = BUTTON_PIN;
    lastReading = HIGH;
    lastState = HIGH;
    lastDebounceTime = 0;
    pressed = false;
}

void Button::begin() {
    pinMode(buttonPin, INPUT_PULLUP);
    Serial.printf("🔘 Button initialized on pin %d\n", buttonPin);
}

void Button::update() {
    int reading = digitalRead(buttonPin);
    
    if (reading != lastReading) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_DELAY) {
        if (reading != lastState) {
            lastState = reading;
            
            // Button pressed (LOW because of INPUT_PULLUP)
            if (reading == LOW) {
                pressed = true;
            }
        }
    }
    
    lastReading = reading;
}

bool Button::wasPressed() {
    if (pressed) {
        pressed = false; // Reset flag
        return true;
    }
    return false;
}

bool Button::isPressed() {
    return digitalRead(buttonPin) == LOW;
}