#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>
#include "config.h"

struct ScheduleInfo {
    int hour;
    int minute;
    int unlockDurationMinutes;
    int weekDay; // 0-6, 0=Sunday (for weekly mode)
    bool isActive;
};

class Timer {
public:
    Timer();
    void begin();
    void start(unsigned long durationMs);
    void stop();
    bool isRunning();
    bool isActive();
    bool wasTriggered();
    unsigned long getTimeRemaining();
    void update();
    
    // Schedule-based methods
    void setDailySchedule(int hour, int minute, int unlockDurationMinutes);
    void setWeeklySchedule(int weekDay, int hour, int minute, int unlockDurationMinutes);
    bool shouldUnlockNow();
    unsigned long getTimeUntilNextScheduledUnlock();
    String getNextUnlockTime();
    
    // Countdown display methods
    String formatTimeRemaining();
    
private:
    unsigned long startTime;
    unsigned long duration;
    bool running;
    bool triggered;
    
    ScheduleInfo schedule;
    unsigned long lastScheduledCheck;
    
    unsigned long getCurrentTimeSeconds();
    unsigned long getSecondsUntilTime(int targetHour, int targetMinute);
    bool isNewDay();
    String formatDuration(unsigned long seconds);
};

#endif // TIMER_H