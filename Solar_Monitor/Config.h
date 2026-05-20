#ifndef CONFIG_H
#define CONFIG_H

// =========================
//This file centralizes Wi-Fi, Supabase, sensor pins, servo pins, I2C power meter settings, tracking control constants, and cleaning detection thresholds.
// =========================

#include <Arduino.h>

// =========================
// Wi-Fi settings
// =========================
static const char* WIFI_SSID = "Wi-Fi Name";
static const char* WIFI_PASSWORD = "password";
static const unsigned long WIFI_RECONNECT_INTERVAL_MS = 10000;

// =========================
// Supabase settings
// =========================
static const char* SUPABASE_URL = "https://mxptqjxphdvnoawkfyut.supabase.co";
static const char* SUPABASE_KEY = "sb_publishable_4HbNTR4F89JNgpwzCTnXHQ_ni8TufDW";

static const char* SUPABASE_READINGS_TABLE = "sensor_readings";
static const char* SUPABASE_COMMANDS_TABLE = "solar_commands";

static const unsigned long DASHBOARD_HTTP_TIMEOUT_MS = 1500;
static const unsigned long DASHBOARD_UPLOAD_INTERVAL_MS = 1000;
static const unsigned long DASHBOARD_UPLOAD_INTERVAL_DURING_LDR_MS = 5000;
static const unsigned long DASHBOARD_COMMAND_POLL_INTERVAL_MS = 1200;
static const unsigned long DASHBOARD_COMMAND_POLL_INTERVAL_DURING_LDR_MS = 5000;

// Perth: UTC+8. NTP provides local time for time-based trajectory control.
static const char* NTP_SERVER = "pool.ntp.org";
static const long GMT_OFFSET_SEC = 8 * 3600;
static const int DAYLIGHT_OFFSET_SEC = 0;

// =========================
// LDR sensor pins
// =========================
static const int PIN_ZUO_XIA   = 34;   // Bottom left
static const int PIN_ZUO_SHANG = 35;   // Top left
static const int PIN_YOU_SHANG = 36;   // Top right
static const int PIN_YOU_XIA   = 39;   // Bottom right

// =========================
// Servo pins
// =========================
static const int SERVO_BASE_PIN  = 25; // Control Left and right
static const int SERVO_PANEL_PIN = 26; // Control Up and Down

// =========================
// I2C power meter settings
// INA219 library is required.
// =========================
static const int I2C_SDA_PIN = 21;
static const int I2C_SCL_PIN = 22;
static const uint8_t INA219_ADDRESS = 0x40;

// SEN0291 specification: 10 mOhm shunt, 0~26 V, +/-8 A.
static const float INA219_SHUNT_OHMS = 0.01f;
static const float INA219_MAX_EXPECTED_CURRENT_A = 8.0f;

// Set to -1.0 if the meter is wired in the opposite current direction.
static const float POWER_CURRENT_DIRECTION = 1.0f;
static const float POWER_CURRENT_CALIBRATION = 1.0f;
static const float POWER_VOLTAGE_CALIBRATION = 1.0f;

// Small-current noise threshold. A 5 mA dead zone prevents jitter around zero.
static const float CURRENT_NOISE_FLOOR_A = 0.005f;
static const float CURRENT_FILTER_ALPHA = 0.45f;
static const int CURRENT_ZERO_RESET_COUNT = 5;

// =========================
// Control parameters
// Keep horizontal tracking more sensitive because side light can produce a
// smaller left/right average difference after the panel frame partially shades sensors.
// =========================
static const int LDR_HORIZONTAL_REACTION = 12;
static const int LDR_VERTICAL_REACTION = 12;

// Sensor refresh interval
static const unsigned long SENSOR_INTERVAL_MS = 1000;

// LDR tracking refresh interval and servo step size per adjustment.
static const unsigned long LDR_TRACKING_INTERVAL_MS = 1;
static const int LDR_HORIZONTAL_STEP_DEG = 1;
static const int LDR_VERTICAL_STEP_DEG = 1;
static const int LDR_TRACKING_MIN_PANEL_ANGLE = 10;
static const int LDR_TRACKING_MAX_PANEL_ANGLE = 160;
static const unsigned long LDR_TRACKING_BURST_MS = 5000;
static const unsigned long LDR_TRACKING_MAX_PERIOD_SEC = 24UL * 60UL * 60UL;
static const bool LDR_INVERT_HORIZONTAL = false;
static const bool LDR_INVERT_VERTICAL = false;

// =========================
// Cleaning detection parameters
// =========================
static const float CLEAN_DROP_RATIO = 0.70f;
static const int BRIGHT_LIGHT_THRESHOLD = 1800;
static const unsigned long DIRTY_CONFIRM_MS = 30000;

static const bool AUTO_TILT_WHEN_DIRTY = false;

// Cleaning angle
static const int CLEAN_PANEL_ANGLE = 180;
static const unsigned long DASHBOARD_COMMAND_MOVE_MS = 5000;

#endif
