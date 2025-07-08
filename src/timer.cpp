#include "timer.h"
#include "servo_control.h"
#include <Arduino.h>

static unsigned long countdownTime = 0;
static bool timerRunning = false;

void startTimer(unsigned long duration) {
    countdownTime = duration;
    timerRunning = true;
}

void stopTimer() {
    timerRunning = false;
}

bool isTimerRunning() {
    return timerRunning;
}

unsigned long getRemainingTime() {
    if (timerRunning) {
        unsigned long currentTime = millis();
        if (currentTime >= countdownTime) {
            timerRunning = false;
            return 0;
        }
        return countdownTime - currentTime;
    }
    return 0;
}

void updateTimer() {
    if (timerRunning) {
        unsigned long remaining = getRemainingTime();
        if (remaining == 0) {
            unlockBox();
        }
    }
}