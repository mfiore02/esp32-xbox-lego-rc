/**
 * Xbox-Lego RC Bridge Configuration
 *
 * Platform: XIAO ESP32-S3
 * Project: BLE Bridge between Xbox One Controller and Lego Technic Move Hub
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// Project Information
// ============================================================================
#define PROJECT_NAME "Xbox-Lego RC Bridge"
#define PROJECT_VERSION "0.1.0"
#define PROJECT_AUTHOR "Your Name"

// ============================================================================
// Hardware Configuration
// ============================================================================
#define BOARD_NAME "XIAO ESP32-S3"

// LED Configuration (XIAO ESP32-S3 has built-in RGB LED)
#define LED_BUILTIN 21  // User LED pin

// ============================================================================
// BLE Configuration
// ============================================================================

// Device Names
#define BLE_DEVICE_NAME "Xbox-Lego-Bridge"
#define XBOX_CONTROLLER_NAME_PREFIX "Xbox"  // Xbox controllers usually advertise as "Xbox Wireless Controller"
#define LEGO_HUB_NAME "Technic Move"        // Lego Technic Move Hub advertised name

// BLE UUIDs - Xbox Controller (HID over GATT)
#define XBOX_HID_SERVICE_UUID            "1812"  // HID Service
#define XBOX_REPORT_CHARACTERISTIC_UUID  "2A4D"  // Report
#define XBOX_REPORT_MAP_UUID             "2A4B"  // Report Map
#define XBOX_BATTERY_SERVICE_UUID        "180F"  // Battery Service
#define XBOX_BATTERY_LEVEL_UUID          "2A19"  // Battery Level

// BLE UUIDs - Lego Technic Move Hub (Custom Protocol)
#define LEGO_SERVICE_UUID "00001623-1212-EFDE-1623-785FEABCD123"
#define LEGO_CHAR_UUID    "00001624-1212-EFDE-1623-785FEABCD123"

// BLE Connection Parameters
#define BLE_SCAN_DURATION 10        // Scan duration in seconds
#define BLE_SCAN_INTERVAL 0x80      // Scan interval (units of 0.625ms)
#define BLE_SCAN_WINDOW   0x30      // Scan window (units of 0.625ms)
#define BLE_CONN_TIMEOUT  5000      // Connection timeout in ms
#define BLE_MAX_RETRIES   3         // Max connection retry attempts

// ============================================================================
// Control Configuration
// ============================================================================

// Default Control Settings
#define DEFAULT_MAX_SPEED_PERCENT   75   // 0-100
#define DEFAULT_DEADZONE_PERCENT    3    // 0-50
#define DEFAULT_TRIGGER_MODE        true // Use triggers for acceleration
#define DEFAULT_INVERT_STEERING     false

// Control Loop Timing
#define CONTROL_LOOP_FREQUENCY_HZ   20   // Control update rate (20Hz = 50ms)
#define CONTROL_LOOP_PERIOD_MS      (1000 / CONTROL_LOOP_FREQUENCY_HZ)

// Display Update Timing
#define DISPLAY_UPDATE_FREQUENCY_HZ 5    // Display update rate (5Hz = 200ms)
#define DISPLAY_UPDATE_PERIOD_MS    (1000 / DISPLAY_UPDATE_FREQUENCY_HZ)

// Serial Update Timing
#define SERIAL_UPDATE_FREQUENCY_HZ 1    // Serial update rate (1Hz = 1s)
#define SERIAL_UPDATE_PERIOD_MS    (1000 / SERIAL_UPDATE_FREQUENCY_HZ)

// ============================================================================
// Lego Hub Protocol Constants
// ============================================================================

// Command Header (9 bytes)
#define LEGO_CMD_HEADER_SIZE 9
static const uint8_t LEGO_CMD_HEADER[LEGO_CMD_HEADER_SIZE] = {
    0x0d, 0x00, 0x81, 0x36, 0x11, 0x51, 0x00, 0x03, 0x00
};

// Total command size
#define LEGO_CMD_TOTAL_SIZE 13

// Light mode values
#define LEGO_LIGHTS_BOTH      0x00  // Front + rear on
#define LEGO_LIGHTS_BRAKE     0x01  // Both on + brake function
#define LEGO_LIGHTS_OFF       0x04  // All off
#define LEGO_LIGHTS_REAR_ONLY 0x05  // Rear only + brake

// Calibration commands
#define LEGO_CALIBRATE_CMD_1  0x10
#define LEGO_CALIBRATE_CMD_2  0x08

// ============================================================================
// Xbox Controller Constants
// ============================================================================

// Analog Input Ranges
#define XBOX_STICK_MIN      -32768
#define XBOX_STICK_MAX       32767
#define XBOX_STICK_CENTER    0
#define XBOX_TRIGGER_MIN     0
#define XBOX_TRIGGER_MAX     1023

// ============================================================================
// Settings Storage (NVS)
// ============================================================================

#define NVS_NAMESPACE "xbox_lego"
#define NVS_KEY_XBOX_ADDR "xbox_addr"
#define NVS_KEY_LEGO_ADDR "lego_addr"
#define NVS_KEY_MAX_SPEED "max_speed"
#define NVS_KEY_DEADZONE  "deadzone"
#define NVS_KEY_TRIGGER_MODE "trig_mode"
#define NVS_KEY_INVERT_STEER "inv_steer"
#define SETTINGS_VERSION 1

// ============================================================================
// Debug Configuration
// ============================================================================

// Enable/disable debug output
#define DEBUG_ENABLED       1
#define DEBUG_BLE           1
#define DEBUG_XBOX          1
#define DEBUG_LEGO          1
#define DEBUG_CONTROLS      1

// Debug macros
#if DEBUG_ENABLED
    #define DEBUG_PRINT(...)   Serial.print(__VA_ARGS__)
    #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
    #define DEBUG_PRINTF(...)  Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(...)
    #define DEBUG_PRINTLN(...)
    #define DEBUG_PRINTF(...)
#endif

#if DEBUG_BLE
    #define DEBUG_BLE_PRINT(...)   Serial.print(__VA_ARGS__)
    #define DEBUG_BLE_PRINTLN(...) Serial.println(__VA_ARGS__)
    #define DEBUG_BLE_PRINTF(...)  Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_BLE_PRINT(...)
    #define DEBUG_BLE_PRINTLN(...)
    #define DEBUG_BLE_PRINTF(...)
#endif

// ============================================================================
// Error Codes
// ============================================================================

enum ErrorCode {
    ERR_NONE = 0,
    ERR_BLE_INIT_FAILED,
    ERR_XBOX_NOT_FOUND,
    ERR_LEGO_NOT_FOUND,
    ERR_XBOX_CONNECT_FAILED,
    ERR_LEGO_CONNECT_FAILED,
    ERR_XBOX_DISCONNECTED,
    ERR_LEGO_DISCONNECTED,
    ERR_SETTINGS_LOAD_FAILED,
    ERR_SETTINGS_SAVE_FAILED
};

#endif // CONFIG_H
