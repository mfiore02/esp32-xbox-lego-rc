/**
 * BLE Manager Implementation
 */

#include "ble_manager.h"

// Global pointer for scan callbacks (NimBLE limitation)
static BLEManager* g_bleManager = nullptr;

// ============================================================================
// BLEManager Implementation
// ============================================================================

BLEManager::BLEManager()
    : xboxClient(nullptr)
    , legoClient(nullptr)
    , xboxState(BLEState::IDLE)
    , legoState(BLEState::IDLE)
    , scanning(false)
{
    g_bleManager = this;
    resetDeviceInfo(xboxInfo);
    resetDeviceInfo(legoInfo);
}

BLEManager::~BLEManager() {
    disconnectAll();
    g_bleManager = nullptr;
}

void BLEManager::init() {
    DEBUG_BLE_PRINTLN("[BLE] Initializing BLE Manager...");

    // Initialize NimBLE
    NimBLEDevice::init(BLE_DEVICE_NAME);

    // Set power level to maximum
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    // Create clients
    xboxClient = NimBLEDevice::createClient();
    legoClient = NimBLEDevice::createClient();

    if (!xboxClient || !legoClient) {
        DEBUG_BLE_PRINTLN("[BLE] ERROR: Failed to create BLE clients!");
        xboxState = BLEState::ERROR;
        legoState = BLEState::ERROR;
        return;
    }

    // Set up client callbacks for disconnect detection
    xboxClient->setClientCallbacks(new ClientCallbacks(this, true));
    legoClient->setClientCallbacks(new ClientCallbacks(this, false));

    DEBUG_BLE_PRINTLN("[BLE] BLE Manager initialized successfully");
    DEBUG_BLE_PRINTF("[BLE] Device name: %s\n", BLE_DEVICE_NAME);
}

void BLEManager::startScan(uint32_t duration) {
    DEBUG_BLE_PRINTLN("[BLE] Starting device scan...");
    DEBUG_BLE_PRINTF("[BLE] Looking for:\n");
    DEBUG_BLE_PRINTF("[BLE]   - Xbox Controller: %s*\n", XBOX_CONTROLLER_NAME_PREFIX);
    DEBUG_BLE_PRINTF("[BLE]   - Lego Hub: %s\n", LEGO_HUB_NAME);

    // Reset device info
    resetDeviceInfo(xboxInfo);
    resetDeviceInfo(legoInfo);

    // Get scan object
    NimBLEScan* pScan = NimBLEDevice::getScan();

    // Set scan callbacks
    pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks(this), true);

    // Configure scan parameters
    pScan->setInterval(BLE_SCAN_INTERVAL);
    pScan->setWindow(BLE_SCAN_WINDOW);
    pScan->setActiveScan(true);  // Active scan uses more power but gets more info

    // Start scanning
    scanning = true;
    xboxState = BLEState::SCANNING;
    legoState = BLEState::SCANNING;

    // Start async scan (non-blocking)
    pScan->start(duration, scanCompleteCB, false);

    DEBUG_BLE_PRINTLN("[BLE] Scan started (asynchronous)");
}

void BLEManager::stopScan() {
    if (scanning) {
        DEBUG_BLE_PRINTLN("[BLE] Stopping scan...");
        NimBLEDevice::getScan()->stop();
        scanning = false;

        // Update states if devices weren't found
        if (!xboxInfo.found && xboxState == BLEState::SCANNING) {
            xboxState = BLEState::IDLE;
        }
        if (!legoInfo.found && legoState == BLEState::SCANNING) {
            legoState = BLEState::IDLE;
        }
    }
}

bool BLEManager::isScanning() {
    return scanning;
}

DeviceInfo BLEManager::getXboxInfo() {
    return xboxInfo;
}

DeviceInfo BLEManager::getLegoInfo() {
    return legoInfo;
}

bool BLEManager::foundXbox() {
    return xboxInfo.found;
}

bool BLEManager::foundLego() {
    return legoInfo.found;
}

bool BLEManager::foundBothDevices() {
    return xboxInfo.found && legoInfo.found;
}

bool BLEManager::connectToXbox() {
    if (!xboxInfo.found) {
        DEBUG_BLE_PRINTLN("[BLE] ERROR: Cannot connect to Xbox - device not found");
        return false;
    }

    DEBUG_BLE_PRINTF("[BLE] Connecting to Xbox controller at %s...\n",
                     xboxInfo.address.toString().c_str());

    xboxState = BLEState::CONNECTING;

    // Attempt connection
    if (xboxClient->connect(xboxInfo.address)) {
        DEBUG_BLE_PRINTLN("[BLE] Connected to Xbox controller!");
        xboxState = BLEState::CONNECTED;
        return true;
    } else {
        DEBUG_BLE_PRINTLN("[BLE] ERROR: Failed to connect to Xbox controller");
        xboxState = BLEState::ERROR;
        return false;
    }
}

bool BLEManager::connectToLego() {
    if (!legoInfo.found) {
        DEBUG_BLE_PRINTLN("[BLE] ERROR: Cannot connect to Lego hub - device not found");
        return false;
    }

    DEBUG_BLE_PRINTF("[BLE] Connecting to Lego hub at %s...\n",
                     legoInfo.address.toString().c_str());

    legoState = BLEState::CONNECTING;

    // Attempt connection
    if (legoClient->connect(legoInfo.address)) {
        DEBUG_BLE_PRINTLN("[BLE] Connected to Lego hub!");
        legoState = BLEState::CONNECTED;
        return true;
    } else {
        DEBUG_BLE_PRINTLN("[BLE] ERROR: Failed to connect to Lego hub");
        legoState = BLEState::ERROR;
        return false;
    }
}

void BLEManager::disconnectXbox() {
    if (xboxClient && xboxClient->isConnected()) {
        DEBUG_BLE_PRINTLN("[BLE] Disconnecting from Xbox controller...");
        xboxClient->disconnect();
        xboxState = BLEState::DISCONNECTED;
    }
}

void BLEManager::disconnectLego() {
    if (legoClient && legoClient->isConnected()) {
        DEBUG_BLE_PRINTLN("[BLE] Disconnecting from Lego hub...");
        legoClient->disconnect();
        legoState = BLEState::DISCONNECTED;
    }
}

void BLEManager::disconnectAll() {
    disconnectXbox();
    disconnectLego();
}

BLEState BLEManager::getXboxState() {
    return xboxState;
}

BLEState BLEManager::getLegoState() {
    return legoState;
}

bool BLEManager::isXboxConnected() {
    return xboxClient && xboxClient->isConnected() && xboxState == BLEState::CONNECTED;
}

bool BLEManager::isLegoConnected() {
    return legoClient && legoClient->isConnected() && legoState == BLEState::CONNECTED;
}

bool BLEManager::areBothConnected() {
    return isXboxConnected() && isLegoConnected();
}

NimBLEClient* BLEManager::getXboxClient() {
    return xboxClient;
}

NimBLEClient* BLEManager::getLegoClient() {
    return legoClient;
}

void BLEManager::setXboxInfo(const DeviceInfo& info) {
    xboxInfo = info;
}

void BLEManager::setLegoInfo(const DeviceInfo& info) {
    legoInfo = info;
}

void BLEManager::handleXboxDisconnect() {
    DEBUG_BLE_PRINTLN("[BLE] !!! Xbox controller DISCONNECTED !!!");
    xboxState = BLEState::DISCONNECTED;
}

void BLEManager::handleLegoDisconnect() {
    DEBUG_BLE_PRINTLN("[BLE] !!! Lego hub DISCONNECTED !!!");
    legoState = BLEState::DISCONNECTED;
}

void BLEManager::resetDeviceInfo(DeviceInfo& info) {
    info.name = "";
    info.address = NimBLEAddress("");
    info.rssi = 0;
    info.found = false;
}

void BLEManager::scanCompleteCB(NimBLEScanResults results) {
    if (g_bleManager) {
        g_bleManager->scanning = false;
        DEBUG_BLE_PRINTLN("[BLE] Scan complete");
        DEBUG_BLE_PRINTF("[BLE] Found %d devices\n", results.getCount());
        DEBUG_BLE_PRINTF("[BLE] Xbox found: %s\n", g_bleManager->foundXbox() ? "YES" : "NO");
        DEBUG_BLE_PRINTF("[BLE] Lego found: %s\n", g_bleManager->foundLego() ? "YES" : "NO");
    }
}

// ============================================================================
// AdvertisedDeviceCallbacks Implementation
// ============================================================================

AdvertisedDeviceCallbacks::AdvertisedDeviceCallbacks(BLEManager* manager)
    : bleManager(manager)
{
}

void AdvertisedDeviceCallbacks::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
    String deviceName = advertisedDevice->getName().c_str();
    NimBLEAddress deviceAddress = advertisedDevice->getAddress();
    int rssi = advertisedDevice->getRSSI();

    DEBUG_BLE_PRINTF("[BLE] Found device: %s (%s) RSSI: %d\n",
                     deviceName.c_str(),
                     deviceAddress.toString().c_str(),
                     rssi);

    // Check if this is an Xbox controller
    if (!bleManager->foundXbox() && deviceName.startsWith(XBOX_CONTROLLER_NAME_PREFIX)) {
        DEBUG_BLE_PRINTLN("[BLE] *** FOUND XBOX CONTROLLER! ***");
        DeviceInfo info;
        info.name = deviceName;
        info.address = deviceAddress;
        info.rssi = rssi;
        info.found = true;

        if (bleManager) {
            bleManager->setXboxInfo(info);
        }
    }

    // Check if this is a Lego hub
    if (!bleManager->foundLego() && deviceName.indexOf(LEGO_HUB_NAME) >= 0) {
        DEBUG_BLE_PRINTLN("[BLE] *** FOUND LEGO HUB! ***");
        DeviceInfo info;
        info.name = deviceName;
        info.address = deviceAddress;
        info.rssi = rssi;
        info.found = true;

        if (bleManager) {
            bleManager->setLegoInfo(info);
        }
    }

    // Stop scan if both devices are found
    if (bleManager && bleManager->foundBothDevices()) {
        DEBUG_BLE_PRINTLN("[BLE] Both devices found! Stopping scan...");
        NimBLEDevice::getScan()->stop();
    }
}

// ============================================================================
// ClientCallbacks Implementation
// ============================================================================

ClientCallbacks::ClientCallbacks(BLEManager* manager, bool isXbox)
    : bleManager(manager)
    , isXboxController(isXbox)
{
}

void ClientCallbacks::onConnect(NimBLEClient* pClient) {
    const char* deviceType = isXboxController ? "Xbox controller" : "Lego hub";
    DEBUG_BLE_PRINTF("[BLE] %s connected (callback)\n", deviceType);
}

void ClientCallbacks::onDisconnect(NimBLEClient* pClient) {
    const char* deviceType = isXboxController ? "Xbox controller" : "Lego hub";
    DEBUG_BLE_PRINTF("[BLE] %s disconnected (callback)\n", deviceType);

    if (bleManager) {
        if (isXboxController) {
            bleManager->handleXboxDisconnect();
        } else {
            bleManager->handleLegoDisconnect();
        }
    }
}
