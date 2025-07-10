#include "servo_control.h"
#include "config.h"
#include <ESP32Servo.h>
#include <Preferences.h>

extern Preferences preferences;

ServoControl::ServoControl() {
    servoPin = SERVO_PIN;
    locked = true;
    currentPosition = 0;
    lockedPosition = SERVO_LOCKED_POSITION;
    unlockedPosition = SERVO_UNLOCKED_POSITION;
}

void ServoControl::begin() {
    servo.attach(servoPin);
    
    // Load calibrated positions from preferences
    lockedPosition = preferences.getInt("servo_locked_pos", SERVO_LOCKED_POSITION);
    unlockedPosition = preferences.getInt("servo_unlocked_pos", SERVO_UNLOCKED_POSITION);
    
    lock(); // Start in locked position
    Serial.printf("🔧 Servo initialized on pin %d (Locked: %d°, Unlocked: %d°)\n", 
                  servoPin, lockedPosition, unlockedPosition);
}

void ServoControl::lock() {
    servo.write(lockedPosition);
    currentPosition = lockedPosition;
    locked = true;
    Serial.printf("🔒 Box locked (position: %d°)\n", lockedPosition);
}

void ServoControl::unlock() {
    servo.write(unlockedPosition);
    currentPosition = unlockedPosition;
    locked = false;
    Serial.printf("🔓 Box unlocked (position: %d°)\n", unlockedPosition);
}

bool ServoControl::isLocked() {
    return locked;
}

void ServoControl::moveTo(int position) {
    if (position >= 0 && position <= 180) {
        servo.write(position);
        currentPosition = position;
        Serial.printf("🎯 Servo moved to %d°\n", position);
        delay(500); // Allow servo to reach position
    }
}

int ServoControl::getCurrentPosition() {
    return currentPosition;
}

void ServoControl::setLockedPosition(int position) {
    if (position >= 0 && position <= 180) {
        lockedPosition = position;
        preferences.putInt("servo_locked_pos", position);
        Serial.printf("🔧 Locked position set to %d°\n", position);
    }
}

void ServoControl::setUnlockedPosition(int position) {
    if (position >= 0 && position <= 180) {
        unlockedPosition = position;
        preferences.putInt("servo_unlocked_pos", position);
        Serial.printf("🔧 Unlocked position set to %d°\n", position);
    }
}

int ServoControl::getLockedPosition() {
    return lockedPosition;
}

int ServoControl::getUnlockedPosition() {
    return unlockedPosition;
}