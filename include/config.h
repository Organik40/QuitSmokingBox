#ifndef CONFIG_H
#define CONFIG_H

// Hardware Pin Definitions
#define OLED_SDA CONFIG_PIN_SDA
#define OLED_SCL CONFIG_PIN_SCL
#define SERVO_PIN CONFIG_PIN_SERVO
#define BUTTON_PIN CONFIG_PIN_BUTTON

// OLED Display Settings
#define SCREEN_WIDTH CONFIG_OLED_WIDTH
#define SCREEN_HEIGHT CONFIG_OLED_HEIGHT
#define OLED_RESET -1        // Reset pin (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // I2C address for OLED

// Servo Settings
#define SERVO_LOCKED_POSITION CONFIG_SERVO_LOCKED_POS
#define SERVO_UNLOCKED_POSITION CONFIG_SERVO_UNLOCKED_POS

// WiFi Settings
#define AP_SSID "QuitSmokingBox"
#define AP_PASSWORD "quit2024"
#define WIFI_TIMEOUT 30000  // 30 seconds

// Timer Settings
#define MIN_TIMER_MINUTES 1
#define MAX_TIMER_MINUTES 1440  // 24 hours
#define DEFAULT_TIMER_MINUTES 30
#define DISPLAY_UPDATE_INTERVAL 1000 // Update interval for the display in milliseconds

// Emergency Settings
#define EMERGENCY_UNLOCK_PENALTY 15  // minutes added to next timer
#define MAX_EMERGENCY_UNLOCKS_PER_DAY 3

// Button Settings
#define BUTTON_DEBOUNCE_DELAY 50 // Debounce delay for button in milliseconds

// Data Storage Keys
#define PREF_NAMESPACE "smoking_box"
#define KEY_TIMER_MODE "timer_mode"
#define KEY_INTERVAL_MINUTES "interval_min"
#define KEY_DAILY_LIMIT "daily_limit"
#define KEY_LAST_UNLOCK "last_unlock"
#define KEY_EMERGENCY_COUNT "emergency_cnt"
#define KEY_TOTAL_CIGARETTES "total_cigs"
#define KEY_DAYS_SMOKE_FREE "smoke_free_days"
#define KEY_DAILY_HOUR "daily_hour"
#define KEY_DAILY_MINUTE "daily_minute"
#define KEY_UNLOCK_DURATION "unlock_duration"
#define KEY_WEEKLY_DAY "weekly_day"
#define KEY_CUSTOM_INTERVALS "custom_intervals"
#define KEY_LAST_SCHEDULED_UNLOCK "last_scheduled"

// Timer Modes
enum TimerMode {
  FIXED_INTERVAL = 0,
  GRADUAL_REDUCTION = 1,
  COMPLETE_QUIT = 2,
  EMERGENCY_ONLY = 3,
  DAILY_SCHEDULE = 4,
  WEEKLY_SCHEDULE = 5,
  CUSTOM_SCHEDULE = 6
};

// Box States
enum BoxState {
  LOCKED = 0,
  UNLOCKED = 1,
  COUNTDOWN = 2,
  EMERGENCY = 3,
  SETUP = 4
};

#endif