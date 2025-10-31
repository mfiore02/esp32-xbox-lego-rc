/**
 * BLE Manager - Handles BLE scanning, connection, and device management
 *
 * Platform: XIAO ESP32-S3
 * Framework: Arduino
 *
 * This module manages BLE operations for both Xbox controller and Lego hub:
 * - Device scanning and discovery
 * - Connection management
 * - Service and characteristic discovery
 * - Connection state monitoring
 */

#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <NimBLEDevice.h>
#include "config.h"

// ============================================================================
// BLE State Enumeration
// ============================================================================

enum class BLEState {
    IDLE,
    SCANNING,
    CONNECTING,
    CONNECTED,
    DISCONNECTED,
    ERROR
};

// ============================================================================
// Device Information Structure
// ============================================================================

struct DeviceInfo {
    String name;
    NimBLEAddress address;
    int rssi;
    bool found;
};

// ============================================================================
// BLE Manager Class
// ============================================================================

class BLEManager {
public:
    BLEManager();
    ~BLEManager();

    // Initialization
    void init();

    // Scanning
    void startScan(uint32_t duration = BLE_SCAN_DURATION);
    void stopScan();
    bool isScanning();

    // Device discovery
    DeviceInfo getXboxInfo();
    DeviceInfo getLegoInfo();
    bool foundXbox();
    bool foundLego();
    bool foundBothDevices();

    // Connection
    bool connectToXbox();
    bool connectToLego();
    void disconnectXbox();
    void disconnectLego();
    void disconnectAll();

    // State queries
    BLEState getXboxState();
    BLEState getLegoState();
    bool isXboxConnected();
    bool isLegoConnected();
    bool areBothConnected();

    // Client getters (for other modules to use)
    NimBLEClient* getXboxClient();
    NimBLEClient* getLegoClient();

    // Device info setters (for callbacks)
    void setXboxInfo(const DeviceInfo& info);
    void setLegoInfo(const DeviceInfo& info);

    // Disconnect handlers (for callbacks)
    void handleXboxDisconnect();
    void handleLegoDisconnect();

private:
    // BLE clients
    NimBLEClient* xboxClient;
    NimBLEClient* legoClient;

    // Device information
    DeviceInfo xboxInfo;
    DeviceInfo legoInfo;

    // State tracking
    BLEState xboxState;
    BLEState legoState;
    bool scanning;

    // Helper functions
    void resetDeviceInfo(DeviceInfo& info);
    static void scanCompleteCB(NimBLEScanResults results);
};

// ============================================================================
// Advertised Device Callbacks
// ============================================================================

class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
public:
    AdvertisedDeviceCallbacks(BLEManager* manager);
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override;

private:
    BLEManager* bleManager;
};

// ============================================================================
// Client Callbacks (for disconnect detection)
// ============================================================================

class ClientCallbacks : public NimBLEClientCallbacks {
public:
    ClientCallbacks(BLEManager* manager, bool isXbox);
    void onConnect(NimBLEClient* pClient) override;
    void onDisconnect(NimBLEClient* pClient) override;

private:
    BLEManager* bleManager;
    bool isXboxController;
};

#endif // BLE_MANAGER_H
