#include "button.h"
#include <Arduino.h>

const int buttonPin = 12; // Define the pin for the button
bool buttonState = false; // Variable to hold the button state

void setupButton() {
    pinMode(buttonPin, INPUT_PULLUP); // Set the button pin as input with pull-up resistor
}

bool isButtonPressed() {
    // Read the button state (LOW when pressed)
    if (digitalRead(buttonPin) == LOW) {
        delay(50); // Debounce delay
        if (digitalRead(buttonPin) == LOW) {
            return true; // Button is pressed
        }
    }
    return false; // Button is not pressed
}