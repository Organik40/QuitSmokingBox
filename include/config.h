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
#define AP_SSID "QuitBox"
#define AP_PASSWORD "Quitin321"
#define WIFI_TIMEOUT 30000  // 30 seconds

// Timer Settings
#define MIN_TIMER_MINUTES 1
#define MAX_TIMER_MINUTES 1440  // 24 hours
#define DEFAULT_TIMER_MINUTES 30
#define DISPLAY_UPDATE_INTERVAL 1000 // Update interval for the display in milliseconds

// Emergency Settings
#define EMERGENCY_UNLOCK_PENALTY 15  // minutes added to next timer
#define MAX_EMERGENCY_UNLOCKS_PER_DAY 3
#define DEFAULT_EMERGENCY_DELAY_MINUTES 5  // cooling-off period for normal mode
#define AI_EMERGENCY_DELAY_MINUTES 10      // AI gatekeeper session minimum time

// AI Emergency Gatekeeper Settings
#define AI_MIN_MESSAGES 5           // Minimum messages before unlock allowed
#define AI_MAX_SESSION_TIME 1800    // 30 minutes max session time

// Statistics and Progress Tracking
#define DEFAULT_CIGARETTE_COST 0.50 // Default cost per cigarette in dollars
#define STATISTICS_UPDATE_INTERVAL 300000 // Update statistics every 5 minutes

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

// Language and Cost Configuration Keys
#define KEY_CURRENT_LANGUAGE "current_language"
#define KEY_SUPPORTED_LANGUAGES "supported_languages"
#define KEY_PRODUCT_NAME "product_name"
#define KEY_CURRENCY "currency"
#define KEY_USE_PACK_PRICE "use_pack_price"
#define KEY_CIGARETTE_COST "cigarette_cost"
#define KEY_PACK_COST "pack_cost"
#define KEY_CIGARETTES_PER_PACK "cigarettes_per_pack"

// Servo Calibration Keys
#define KEY_SERVO_LOCKED_POS "servo_locked_pos"
#define KEY_SERVO_UNLOCKED_POS "servo_unlocked_pos"

// Network Security Keys
#define KEY_BLOCK_ON_PUBLIC "block_on_public"
#define KEY_ALLOWED_NETWORKS "allowed_networks"
#define KEY_BLOCKED_NETWORKS "blocked_networks"

// AI Configuration Keys
#define KEY_AI_ENABLED "ai_enabled"
#define KEY_AI_PROVIDER "ai_provider"
#define KEY_AI_PERSONALITY "ai_personality"
#define KEY_AI_DELAY_MINUTES "ai_delay_minutes"
#define KEY_AI_API_KEY "ai_api_key"

// Progress Tracking Keys
#define KEY_SMOKING_GOAL "smoking_goal"
#define KEY_START_DATE "start_date"
#define KEY_TARGET_DATE "target_date"
#define KEY_MONEY_SAVED "money_saved"
#define KEY_LONGEST_STREAK "longest_streak"
#define KEY_TOTAL_DAYS "total_days"

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