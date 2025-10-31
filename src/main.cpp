/**
 * Xbox-Lego RC Bridge - Main Application
 *
 * Platform: XIAO ESP32-S3
 * Framework: Arduino
 *
 * This application bridges an Xbox One controller to a Lego Technic Move Hub
 * using Bluetooth Low Energy (BLE).
 */

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "config.h"

// ============================================================================
// Global Variables
// ============================================================================

// Application state
enum class AppState {
    INIT,
    SCANNING,
    CONNECTING,
    CONNECTED,
    ACTIVE,
    ERROR
};

AppState currentState = AppState::INIT;
unsigned long lastControlUpdate = 0;
unsigned long lastDisplayUpdate = 0;

// ============================================================================
// Function Prototypes
// ============================================================================

void setup();
void loop();
void initBLE();
void startScanning();
void updateControlLoop();
void updateDisplay();
void handleError(ErrorCode error);

// ============================================================================
// Setup - Runs once at startup
// ============================================================================

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);  // Wait for serial to initialize

    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN(PROJECT_NAME);
    DEBUG_PRINTLN("Version: " PROJECT_VERSION);
    DEBUG_PRINTLN("Board: " BOARD_NAME);
    DEBUG_PRINTLN("========================================");

    // Initialize built-in LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Initialize BLE
    DEBUG_PRINTLN("\n[INIT] Initializing BLE...");
    initBLE();

    // TODO: Load settings from NVS
    DEBUG_PRINTLN("[INIT] Loading settings...");

    // Set initial state
    currentState = AppState::SCANNING;
    DEBUG_PRINTLN("[INIT] Initialization complete");
    DEBUG_PRINTLN("[SCAN] Starting device scan...\n");

    // Start scanning for devices
    startScanning();
}

// ============================================================================
// Loop - Runs continuously
// ============================================================================

void loop() {
    unsigned long currentMillis = millis();
    static unsigned long lastDisplayUpdate = 0;
    
    if (lastDisplayUpdate < (currentMillis - DISPLAY_UPDATE_PERIOD_MS))
    {
        updateDisplay();
        lastDisplayUpdate = currentMillis;
    }

    // State machine
    switch (currentState) {
        case AppState::INIT:
            // Initialization handled in setup()
            break;

        case AppState::SCANNING:
            // Scanning is handled by BLE callbacks
            // Just blink LED to show we're alive
            static unsigned long lastBlink = 0;
            if (currentMillis - lastBlink > 500) {
                digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
                lastBlink = currentMillis;
            }
            break;

        case AppState::CONNECTING:
            // Connection is handled by BLE callbacks
            digitalWrite(LED_BUILTIN, HIGH);  // Solid LED during connection
            break;

        case AppState::CONNECTED:
            // Devices connected, ready to start control loop
            DEBUG_PRINTLN("[STATE] Devices connected! Starting control loop...");
            currentState = AppState::ACTIVE;
            digitalWrite(LED_BUILTIN, HIGH);
            break;

        case AppState::ACTIVE:
            // Main control loop
            if (currentMillis - lastControlUpdate >= CONTROL_LOOP_PERIOD_MS) {
                updateControlLoop();
                lastControlUpdate = currentMillis;
            }

            // Display update (less frequent)
            if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_PERIOD_MS) {
                lastDisplayUpdate = currentMillis;
            }
            break;

        case AppState::ERROR:
            // Error state - blink LED rapidly
            static unsigned long lastErrorBlink = 0;
            if (currentMillis - lastErrorBlink > 100) {
                digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
                lastErrorBlink = currentMillis;
            }
            break;
    }

    // Small delay to prevent watchdog issues
    delay(1);
}

// ============================================================================
// BLE Initialization
// ============================================================================

void initBLE() {
    DEBUG_PRINTLN("[BLE] Initializing NimBLE stack...");

    // Initialize NimBLE
    NimBLEDevice::init(BLE_DEVICE_NAME);

    // Set power level
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // Maximum power

    DEBUG_PRINTLN("[BLE] NimBLE initialized successfully");

    // TODO: Create BLE client instances
    // TODO: Set up scan parameters
}

// ============================================================================
// Start BLE Scanning
// ============================================================================

void startScanning() {
    DEBUG_PRINTLN("[SCAN] Starting BLE scan...");
    DEBUG_PRINTF("[SCAN] Looking for:\n");
    DEBUG_PRINTF("  - Xbox Controller: %s*\n", XBOX_CONTROLLER_NAME_PREFIX);
    DEBUG_PRINTF("  - Lego Hub: %s\n", LEGO_HUB_NAME);
    DEBUG_PRINTLN();

    // TODO: Implement BLE scanning
    // This will be implemented when we add BLE manager module

    DEBUG_PRINTLN("[SCAN] BLE scanning active...");
}

// ============================================================================
// Control Loop Update
// ============================================================================

void updateControlLoop() {
    // TODO: Read Xbox controller state
    // TODO: Map inputs to Lego controls
    // TODO: Send commands to Lego hub

    // Placeholder
    static int counter = 0;
    if (++counter % 20 == 0) {  // Print every 20 updates (1 second at 20Hz)
        DEBUG_PRINTF("[CTRL] Control loop running... (%d updates)\n", counter);
    }
}

// ============================================================================
// Display Update
// ============================================================================

void updateDisplay() {
    // TODO: Update OLED display (if present)
    // TODO: Print status to serial

    // Placeholder - print simple status
    static int displayCounter = 0;
    if (++displayCounter % 5 == 0) {  // Every 5 updates (5 seconds at 1Hz)
        DEBUG_PRINTLN("\n========== Status ==========");
        DEBUG_PRINTF("State: %s\n",
            currentState == AppState::INIT ? "INIT" :
            currentState == AppState::SCANNING ? "SCANNING" :
            currentState == AppState::CONNECTING ? "CONNECTING" :
            currentState == AppState::CONNECTED ? "CONNECTED" :
            currentState == AppState::ACTIVE ? "ACTIVE" : "ERROR");
        DEBUG_PRINTF("Uptime: %lu seconds\n", millis() / 1000);
        DEBUG_PRINTLN("============================\n");
    }
}

// ============================================================================
// Error Handling
// ============================================================================

void handleError(ErrorCode error) {
    currentState = AppState::ERROR;

    DEBUG_PRINTLN("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    DEBUG_PRINTF("ERROR: Code %d\n", error);

    switch (error) {
        case ERR_BLE_INIT_FAILED:
            DEBUG_PRINTLN("BLE initialization failed");
            break;
        case ERR_XBOX_NOT_FOUND:
            DEBUG_PRINTLN("Xbox controller not found");
            break;
        case ERR_LEGO_NOT_FOUND:
            DEBUG_PRINTLN("Lego hub not found");
            break;
        case ERR_XBOX_CONNECT_FAILED:
            DEBUG_PRINTLN("Failed to connect to Xbox controller");
            break;
        case ERR_LEGO_CONNECT_FAILED:
            DEBUG_PRINTLN("Failed to connect to Lego hub");
            break;
        case ERR_XBOX_DISCONNECTED:
            DEBUG_PRINTLN("Xbox controller disconnected");
            break;
        case ERR_LEGO_DISCONNECTED:
            DEBUG_PRINTLN("Lego hub disconnected");
            break;
        default:
            DEBUG_PRINTLN("Unknown error");
            break;
    }

    DEBUG_PRINTLN("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}
