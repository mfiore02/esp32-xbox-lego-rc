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
#include "ble_manager.h"

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

// BLE Manager instance
BLEManager* bleManager = nullptr;

// ============================================================================
// Function Prototypes
// ============================================================================

void setup();
void loop();
void initBLE();
void startScanning();
void updateControlLoop();
void updateDisplay();
void updateSerial();
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
    
    static unsigned long lastSerialUpdate = 0;
    if (lastSerialUpdate < (currentMillis - SERIAL_UPDATE_PERIOD_MS))
    {
        updateSerial();
        lastSerialUpdate = currentMillis;
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

            // Check if scan is complete and devices found
            if (bleManager && !bleManager->isScanning()) {
                if (bleManager->foundBothDevices()) {
                    DEBUG_PRINTLN("\n[SCAN] Both devices found! Attempting connections...");
                    currentState = AppState::CONNECTING;
                } else {
                    // Scan complete but devices not found
                    DEBUG_PRINTLN("\n[SCAN] Scan complete - devices missing:");
                    if (!bleManager->foundXbox()) {
                        DEBUG_PRINTLN("[SCAN]   - Xbox controller NOT FOUND");
                    }
                    if (!bleManager->foundLego()) {
                        DEBUG_PRINTLN("[SCAN]   - Lego hub NOT FOUND");
                    }
                    DEBUG_PRINTLN("[SCAN] Restarting scan in 3 seconds...");
                    delay(3000);
                    startScanning();
                }
            }
            break;

        case AppState::CONNECTING: {
            // Solid LED during connection
            digitalWrite(LED_BUILTIN, HIGH);

            // Attempt to connect to both devices
            static bool connectingStarted = false;
            if (!connectingStarted) {
                connectingStarted = true;

                DEBUG_PRINTLN("\n[CONN] Connecting to Xbox controller...");
                bool xboxConnected = bleManager->connectToXbox();

                if (xboxConnected) {
                    DEBUG_PRINTLN("[CONN] Xbox controller connected!");
                    // Give BLE stack time to stabilize before second connection
                    DEBUG_PRINTLN("[CONN] Waiting for BLE stack to stabilize...");
                    delay(1000);  // 1 second delay
                } else {
                    DEBUG_PRINTLN("[CONN] ERROR: Failed to connect to Xbox controller");
                    handleError(ERR_XBOX_CONNECT_FAILED);
                    connectingStarted = false;
                    break;
                }

                DEBUG_PRINTLN("\n[CONN] Connecting to Lego hub...");
                bool legoConnected = bleManager->connectToLego();

                if (legoConnected) {
                    DEBUG_PRINTLN("[CONN] Lego hub connected!");
                    currentState = AppState::CONNECTED;
                } else {
                    DEBUG_PRINTLN("[CONN] ERROR: Failed to connect to Lego hub");
                    handleError(ERR_LEGO_CONNECT_FAILED);
                }

                connectingStarted = false;
            }
            break;
        }

        case AppState::CONNECTED:
            // Devices connected, ready to start control loop
            DEBUG_PRINTLN("[STATE] Devices connected! Starting control loop...");
            currentState = AppState::ACTIVE;
            digitalWrite(LED_BUILTIN, HIGH);
            break;

        case AppState::ACTIVE:
            // Check for disconnections
            if (bleManager) {
                if (!bleManager->isXboxConnected()) {
                    DEBUG_PRINTLN("\n[ERROR] Xbox controller disconnected!");
                    handleError(ERR_XBOX_DISCONNECTED);
                    break;
                }
                if (!bleManager->isLegoConnected()) {
                    DEBUG_PRINTLN("\n[ERROR] Lego hub disconnected!");
                    handleError(ERR_LEGO_DISCONNECTED);
                    break;
                }
            }

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
    DEBUG_PRINTLN("[BLE] Initializing BLE Manager...");

    // Create BLE manager instance
    bleManager = new BLEManager();

    if (!bleManager) {
        DEBUG_PRINTLN("[BLE] ERROR: Failed to create BLE manager!");
        handleError(ERR_BLE_INIT_FAILED);
        return;
    }

    // Initialize BLE manager
    bleManager->init();

    DEBUG_PRINTLN("[BLE] BLE Manager initialized successfully");
}

// ============================================================================
// Start BLE Scanning
// ============================================================================

void startScanning() {
    if (!bleManager) {
        DEBUG_PRINTLN("[SCAN] ERROR: BLE manager not initialized!");
        handleError(ERR_BLE_INIT_FAILED);
        return;
    }

    DEBUG_PRINTLN("[SCAN] Starting device scan...");
    bleManager->startScan(BLE_SCAN_DURATION);
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
}

void updateSerial() {
    DEBUG_PRINTLN("\n========== Status ==========");
    DEBUG_PRINTF("State: %s\n",
        currentState == AppState::INIT ? "INIT" :
        currentState == AppState::SCANNING ? "SCANNING" :
        currentState == AppState::CONNECTING ? "CONNECTING" :
        currentState == AppState::CONNECTED ? "CONNECTED" :
        currentState == AppState::ACTIVE ? "ACTIVE" : "ERROR");
    DEBUG_PRINTF("Uptime: %lu seconds\n", millis() / 1000);

    if (bleManager) {
        DEBUG_PRINTLN("--- BLE Status ---");
        if (bleManager->foundXbox()) {
            DeviceInfo xbox = bleManager->getXboxInfo();
            const char* xboxStatus = bleManager->isXboxConnected() ? "CONNECTED" : "disconnected";
            DEBUG_PRINTF("Xbox: %s [%s]\n", xbox.name.c_str(), xboxStatus);
            DEBUG_PRINTF("  Address: %s, RSSI: %d\n",
                        xbox.address.toString().c_str(),
                        xbox.rssi);
        } else {
            DEBUG_PRINTLN("Xbox: Not found");
        }

        if (bleManager->foundLego()) {
            DeviceInfo lego = bleManager->getLegoInfo();
            const char* legoStatus = bleManager->isLegoConnected() ? "CONNECTED" : "disconnected";
            DEBUG_PRINTF("Lego: %s [%s]\n", lego.name.c_str(), legoStatus);
            DEBUG_PRINTF("  Address: %s, RSSI: %d\n",
                        lego.address.toString().c_str(),
                        lego.rssi);
        } else {
            DEBUG_PRINTLN("Lego: Not found");
        }
    }
    DEBUG_PRINTLN("============================\n");
}

// ============================================================================
// Error Handling
// ============================================================================

void handleError(ErrorCode error) {
    DEBUG_PRINTLN("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    DEBUG_PRINTF("ERROR: Code %d\n", error);

    switch (error) {
        case ERR_BLE_INIT_FAILED:
            DEBUG_PRINTLN("BLE initialization failed");
            currentState = AppState::ERROR;
            break;
        case ERR_XBOX_NOT_FOUND:
            DEBUG_PRINTLN("Xbox controller not found");
            currentState = AppState::ERROR;
            break;
        case ERR_LEGO_NOT_FOUND:
            DEBUG_PRINTLN("Lego hub not found");
            currentState = AppState::ERROR;
            break;
        case ERR_XBOX_CONNECT_FAILED:
            DEBUG_PRINTLN("Failed to connect to Xbox controller");
            currentState = AppState::ERROR;
            break;
        case ERR_LEGO_CONNECT_FAILED:
            DEBUG_PRINTLN("Failed to connect to Lego hub");
            currentState = AppState::ERROR;
            break;
        case ERR_XBOX_DISCONNECTED:
            DEBUG_PRINTLN("Xbox controller disconnected");
            DEBUG_PRINTLN("Will attempt reconnection...");
            if (bleManager) {
                bleManager->resetForReconnection();
            }
            delay(2000);  // Wait 2 seconds before rescanning
            currentState = AppState::SCANNING;
            startScanning();
            break;
        case ERR_LEGO_DISCONNECTED:
            DEBUG_PRINTLN("Lego hub disconnected");
            DEBUG_PRINTLN("Will attempt reconnection...");
            if (bleManager) {
                bleManager->resetForReconnection();
            }
            delay(2000);  // Wait 2 seconds before rescanning
            currentState = AppState::SCANNING;
            startScanning();
            break;
        default:
            DEBUG_PRINTLN("Unknown error");
            currentState = AppState::ERROR;
            break;
    }

    DEBUG_PRINTLN("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}
