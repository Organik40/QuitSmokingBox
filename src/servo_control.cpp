#include "servo_control.h"
#include "config.h"
#include <ESP32Servo.h>

ServoControl::ServoControl() {
    servoPin = SERVO_PIN;
    locked = true;
}

void ServoControl::begin() {
    servo.attach(servoPin);
    lock(); // Start in locked position
    Serial.printf("🔧 Servo initialized on pin %d\n", servoPin);
}

void ServoControl::lock() {
    servo.write(SERVO_LOCKED_POSITION);
    locked = true;
    Serial.println("🔒 Box locked");
}

void ServoControl::unlock() {
    servo.write(SERVO_UNLOCKED_POSITION);
    locked = false;
    Serial.println("🔓 Box unlocked");
}

bool ServoControl::isLocked() {
    return locked;
}