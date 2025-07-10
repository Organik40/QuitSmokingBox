#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoControl {
public:
    ServoControl();
    void begin();
    void lock();
    void unlock();
    bool isLocked();
    void moveTo(int position);
    int getCurrentPosition();
    void setLockedPosition(int position);
    void setUnlockedPosition(int position);
    int getLockedPosition();
    int getUnlockedPosition();

private:
    Servo servo;
    int servoPin;
    bool locked;
    int currentPosition;
    int lockedPosition;
    int unlockedPosition;
};

#endif // SERVO_CONTROL_H