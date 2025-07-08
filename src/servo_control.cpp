#include "servo_control.h"
#include <Servo.h>

Servo lockServo;

void initServo(int pin) {
    lockServo.attach(pin);
    lockServo.write(0); // Start in locked position
}

void lockBox() {
    lockServo.write(0); // Lock position
}

void unlockBox() {
    lockServo.write(90); // Unlock position
}