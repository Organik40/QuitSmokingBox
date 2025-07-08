#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>

class ServoControl {
public:
    ServoControl(int servoPin);
    void lock();
    void unlock();
    bool isLocked();

private:
    int servoPin;
    bool locked;
};

#endif // SERVO_CONTROL_H