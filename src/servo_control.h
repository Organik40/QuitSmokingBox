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

private:
    Servo servo;
    int servoPin;
    bool locked;
};

#endif // SERVO_CONTROL_H