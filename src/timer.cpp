#include "timer.h"
#include <Preferences.h>
#include <time.h>

extern Preferences preferences;

Timer::Timer() {
    startTime = 0;
    duration = 0;
    running = false;
    triggered = false;
    lastScheduledCheck = 0;
    
    // Initialize schedule
    schedule.hour = 22; // Default 22:00 (10 PM)
    schedule.minute = 0;
    schedule.unlockDurationMinutes = 30;
    schedule.weekDay = 0; // Sunday
    schedule.isActive = false;
}

void Timer::begin() {
    Serial.println("⏱️ Timer system initialized");
    // Load schedule from preferences if needed
    // This is called after preferences are initialized
}

void Timer::start(unsigned long durationMs) {
    startTime = millis();
    duration = durationMs;
    running = true;
    triggered = false;
    Serial.printf("⏱️ Timer started for %lu ms\n", durationMs);
}

void Timer::stop() {
    running = false;
    triggered = false;
    Serial.println("⏱️ Timer stopped");
}

bool Timer::wasTriggered() {
    if (triggered) {
        triggered = false; // Reset flag after checking
        return true;
    }
    return false;
}

bool Timer::isRunning() {
    if (running) {
        if (millis() - startTime >= duration) {
            running = false;
            return false;
        }
    }
    return running;
}

bool Timer::isActive() {
    return isRunning() || schedule.isActive;
}

unsigned long Timer::getTimeRemaining() {
    if (!running) return 0;
    
    unsigned long elapsed = millis() - startTime;
    if (elapsed >= duration) {
        running = false;
        return 0;
    }
    return duration - elapsed;
}

void Timer::update() {
    // Check if timer expired
    if (running && millis() - startTime >= duration) {
        running = false;
        triggered = true;
        Serial.println("⏰ Timer expired - box should unlock");
    }
    
    // Check scheduled unlocks
    if (schedule.isActive) {
        unsigned long currentTime = millis();
        // Only check once per minute to avoid excessive processing
        if (currentTime - lastScheduledCheck >= 60000) {
            lastScheduledCheck = currentTime;
            
            if (shouldUnlockNow()) {
                triggered = true;
                Serial.println("📅 Scheduled unlock triggered");
            }
        }
    }
}

void Timer::setDailySchedule(int hour, int minute, int unlockDurationMinutes) {
    schedule.hour = hour;
    schedule.minute = minute;
    schedule.unlockDurationMinutes = unlockDurationMinutes;
    schedule.isActive = true;
    
    // Save to preferences
    preferences.putInt(KEY_DAILY_HOUR, hour);
    preferences.putInt(KEY_DAILY_MINUTE, minute);
    preferences.putInt(KEY_UNLOCK_DURATION, unlockDurationMinutes);
    
    Serial.printf("📅 Daily schedule set: %02d:%02d for %d minutes\n", hour, minute, unlockDurationMinutes);
}

void Timer::setWeeklySchedule(int weekDay, int hour, int minute, int unlockDurationMinutes) {
    schedule.weekDay = weekDay;
    schedule.hour = hour;
    schedule.minute = minute;
    schedule.unlockDurationMinutes = unlockDurationMinutes;
    schedule.isActive = true;
    
    // Save to preferences
    preferences.putInt(KEY_WEEKLY_DAY, weekDay);
    preferences.putInt(KEY_DAILY_HOUR, hour);
    preferences.putInt(KEY_DAILY_MINUTE, minute);
    preferences.putInt(KEY_UNLOCK_DURATION, unlockDurationMinutes);
    
    const char* dayNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    Serial.printf("📅 Weekly schedule set: %s at %02d:%02d for %d minutes\n", 
                  dayNames[weekDay], hour, minute, unlockDurationMinutes);
}

bool Timer::shouldUnlockNow() {
    if (!schedule.isActive) return false;
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("⚠️ Failed to get local time");
        return false;
    }
    
    TimerMode currentMode = (TimerMode)preferences.getInt(KEY_TIMER_MODE, FIXED_INTERVAL);
    unsigned long lastScheduledUnlock = preferences.getULong64(KEY_LAST_SCHEDULED_UNLOCK, 0);
    unsigned long currentTime = getCurrentTimeSeconds();
    
    // Check if we already unlocked today/this week
    if (currentMode == DAILY_SCHEDULE) {
        // Check if we already unlocked today
        if (currentTime - lastScheduledUnlock < 86400) { // Less than 24 hours
            return false;
        }
        
        // Check if it's the right time
        if (timeinfo.tm_hour == schedule.hour && timeinfo.tm_min == schedule.minute) {
            preferences.putULong64(KEY_LAST_SCHEDULED_UNLOCK, currentTime);
            return true;
        }
    } else if (currentMode == WEEKLY_SCHEDULE) {
        // Check if it's the right day and we haven't unlocked this week
        if (timeinfo.tm_wday == schedule.weekDay) {
            // Check if we already unlocked this week
            if (currentTime - lastScheduledUnlock < 604800) { // Less than 7 days
                return false;
            }
            
            // Check if it's the right time
            if (timeinfo.tm_hour == schedule.hour && timeinfo.tm_min == schedule.minute) {
                preferences.putULong64(KEY_LAST_SCHEDULED_UNLOCK, currentTime);
                return true;
            }
        }
    }
    
    return false;
}

unsigned long Timer::getTimeUntilNextScheduledUnlock() {
    if (!schedule.isActive) return 0;
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return 0;
    }
    
    TimerMode currentMode = (TimerMode)preferences.getInt(KEY_TIMER_MODE, FIXED_INTERVAL);
    
    if (currentMode == DAILY_SCHEDULE) {
        return getSecondsUntilTime(schedule.hour, schedule.minute);
    } else if (currentMode == WEEKLY_SCHEDULE) {
        // Calculate seconds until next scheduled day
        int currentDay = timeinfo.tm_wday;
        int targetDay = schedule.weekDay;
        
        int daysUntil;
        if (targetDay > currentDay) {
            daysUntil = targetDay - currentDay;
        } else if (targetDay < currentDay) {
            daysUntil = 7 - (currentDay - targetDay);
        } else {
            // Same day - check if time has passed
            int currentTimeMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
            int targetTimeMinutes = schedule.hour * 60 + schedule.minute;
            
            if (targetTimeMinutes > currentTimeMinutes) {
                daysUntil = 0;
            } else {
                daysUntil = 7; // Next week
            }
        }
        
        unsigned long secondsUntilDay = daysUntil * 86400;
        unsigned long secondsUntilTime = getSecondsUntilTime(schedule.hour, schedule.minute);
        
        if (daysUntil == 0) {
            return secondsUntilTime;
        } else {
            // Calculate time to start of target day, then add time to target hour
            unsigned long secondsToMidnight = getSecondsUntilTime(0, 0);
            return secondsToMidnight + (daysUntil - 1) * 86400 + schedule.hour * 3600 + schedule.minute * 60;
        }
    }
    
    return 0;
}

String Timer::getNextUnlockTime() {
    unsigned long secondsUntil = getTimeUntilNextScheduledUnlock();
    if (secondsUntil == 0) return "Not scheduled";
    
    return formatDuration(secondsUntil);
}

String Timer::formatTimeRemaining() {
    if (running) {
        return formatDuration(getTimeRemaining() / 1000);
    } else if (schedule.isActive) {
        return formatDuration(getTimeUntilNextScheduledUnlock());
    }
    return "00:00:00";
}

unsigned long Timer::getCurrentTimeSeconds() {
    return time(nullptr);
}

unsigned long Timer::getSecondsUntilTime(int targetHour, int targetMinute) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return 0;
    }
    
    int currentMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
    int targetMinutes = targetHour * 60 + targetMinute;
    
    int minutesUntil;
    if (targetMinutes > currentMinutes) {
        minutesUntil = targetMinutes - currentMinutes;
    } else {
        minutesUntil = 1440 - (currentMinutes - targetMinutes); // Next day
    }
    
    return minutesUntil * 60 - timeinfo.tm_sec; // Subtract current seconds for precision
}

String Timer::formatDuration(unsigned long seconds) {
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;
    
    char buffer[16];
    if (hours > 0) {
        snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, secs);
    } else {
        snprintf(buffer, sizeof(buffer), "%02lu:%02lu", minutes, secs);
    }
    
    return String(buffer);
}