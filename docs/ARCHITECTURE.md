# Xbox-Lego RC BLE Bridge Architecture

## Platform: XIAO ESP32-S3

This project uses the **Seeed XIAO ESP32-S3** as a BLE bridge between an Xbox One controller and a Lego Technic Move Hub, leveraging the ESP32's mature BLE stack and existing libraries for rapid development.

---

## Overview
This ESP32-S3 application bridges an Xbox One controller (model 1914) to a Lego Technic Move Hub (42176 Porsche GT4 e-performance) via Bluetooth Low Energy, translating HID over GATT controller inputs into Lego RC car control commands.

## System Architecture

### High-Level Components

```
┌─────────────────────┐         ┌──────────────────────┐         ┌─────────────────────┐
│  Xbox One           │         │  XIAO ESP32-S3       │         │  Lego Move Hub      │
│  Controller         │  BLE    │  BLE Bridge          │  BLE    │  (42176)            │
│  (Model 1914)       │────────▶│                      │────────▶│                     │
│                     │         │  - Input Handler     │         │  - Motor Control    │
│  HID over GATT      │         │  - Control Mapper    │         │  - Steering         │
│                     │         │  - Speed Limiter     │         │  - Lights           │
└─────────────────────┘         └──────────────────────┘         └─────────────────────┘
```

## Module Structure

### 1. Core Application (`main.cpp`)
**Responsibilities:**
- Application lifecycle management
- Main event loop
- Module coordination
- Optional display management (if OLED added)

**Key Functions:**
```cpp
void setup();
void loop();
void app_init();
void connect_devices();
void update_control_loop();
```

### 2. BLE Manager Module (`ble_manager.cpp/h`)
**Responsibilities:**
- NimBLE stack initialization
- Device scanning and discovery
- Connection management for both devices
- GATT service/characteristic discovery
- Connection state monitoring

**Libraries Used:**
- NimBLE-Arduino (BLE stack)
- ESP32-BLE-HID-exp (Xbox controller support)

**Key Classes:**
```cpp
enum class BLEState {
    Idle,
    Scanning,
    Connecting,
    Connected,
    Error
};

class BLEManager {
public:
    void init();
    void startScan();
    bool connectXbox(NimBLEAddress address);
    bool connectLego(NimBLEAddress address);
    void disconnectAll();
    BLEState getXboxState();
    BLEState getLegoState();

private:
    NimBLEClient* xboxClient;
    NimBLEClient* legoClient;
    BLEState xboxState;
    BLEState legoState;
};
```

### 3. Xbox Controller Interface (`xbox_controller.cpp/h`)
**Responsibilities:**
- HID over GATT profile handling
- Input report parsing (using ESP32-BLE-HID-exp library)
- Button/axis state tracking
- Battery level monitoring

**Protocol Details:**
- Service UUID: `0x1812` (HID Service)
- Report Characteristic: `0x2A4D`
- Report Map Characteristic: `0x2A4B`
- Battery Service: `0x180F`
- Battery Level Characteristic: `0x2A19`

**Key Classes:**
```cpp
class XboxControllerState {
public:
    // D-pad and buttons
    bool btn_a, btn_b, btn_x, btn_y;
    bool btn_lb, btn_rb;
    bool btn_view, btn_menu, btn_xbox;
    bool dpad_up, dpad_down, dpad_left, dpad_right;

    // Analog inputs (-32768 to 32767)
    int16_t left_stick_x, left_stick_y;
    int16_t right_stick_x, right_stick_y;

    // Triggers (0 to 1023)
    uint16_t left_trigger, right_trigger;

    // Battery
    uint8_t battery_level; // 0-100
};

class XboxController {
public:
    void init(NimBLEClient* client);
    void parseReport(const uint8_t* data, size_t len);
    XboxControllerState getState();
    uint8_t getBatteryLevel();

private:
    NimBLEClient* bleClient;
    XboxControllerState state;
};
```

### 4. Lego Hub Interface (`lego_hub.cpp/h`)
**Responsibilities:**
- Lego Technic Move Hub (88019) protocol implementation
- Command formatting and transmission
- Motor control
- Steering calibration
- Light control

**Implementation Options:**
1. Use Legoino library (if compatible with 88019)
2. Direct BLE implementation using known protocol

**Protocol Details:**
- Service UUID: `00001623-1212-EFDE-1623-785FEABCD123`
- Characteristic UUID: `00001624-1212-EFDE-1623-785FEABCD123`
- Command Format: 13-byte fixed structure

**Key Classes:**
```cpp
enum class LegoLightMode {
    Both = 0x00,       // Front + rear on
    Brake = 0x01,      // Both on + brake function
    Off = 0x04,        // All off
    RearOnly = 0x05    // Rear only + brake
};

class LegoHub {
public:
    void init(NimBLEClient* client);
    void calibrateSteering();
    void sendControl(int8_t speed, int8_t steering, LegoLightMode lights);
    void emergencyStop();
    bool isCalibrated();

private:
    NimBLEClient* bleClient;
    NimBLERemoteCharacteristic* controlChar;
    int8_t currentSpeed;
    int8_t currentSteering;
    bool calibrated;

    static constexpr uint8_t CMD_HEADER[] =
        {0x0d, 0x00, 0x81, 0x36, 0x11, 0x51, 0x00, 0x03, 0x00};
};
```

### 5. Control Mapper Module (`control_mapper.cpp/h`)
**Responsibilities:**
- Map Xbox controller inputs to Lego car controls
- Apply speed limiting
- Handle control curves and dead zones
- Implement control modes

**Key Classes:**
```cpp
struct ControlSettings {
    uint8_t maxSpeedPercent = 75;  // 0-100
    uint8_t deadzonePercent = 3;   // 0-50
    bool triggerAcceleration = true; // Use triggers vs stick for speed
    bool invertSteering = false;
};

struct MappedControls {
    int8_t speed;       // -100 to 100
    int8_t steering;    // -100 to 100
    LegoLightMode lights;
};

class ControlMapper {
public:
    ControlMapper(ControlSettings settings);
    MappedControls map(const XboxControllerState& xboxState);
    void updateSettings(ControlSettings settings);

private:
    ControlSettings settings;
    int8_t applyDeadzone(int16_t value);
    int8_t applySpeedLimit(int8_t speed);
};
```

**Mapping Logic:**
- **Speed:** Right trigger (forward) / Left trigger (backward) OR left stick Y-axis
- **Steering:** Left stick X-axis
- **Lights:**
  - A button: Toggle lights on/off
  - B button: Cycle light modes
  - Right bumper: Brake lights
- **Emergency stop:** B button + X button simultaneously

### 6. Settings Manager (`settings.cpp/h`)
**Responsibilities:**
- Persistent storage using ESP32 Preferences library
- Device address storage
- Control configuration storage

**Key Classes:**
```cpp
class SettingsManager {
public:
    void init();
    bool loadSettings();
    bool saveSettings();
    void resetToDefaults();

    // Device addresses
    bool hasXboxAddress();
    bool hasLegoAddress();
    NimBLEAddress getXboxAddress();
    NimBLEAddress getLegoAddress();
    void setXboxAddress(NimBLEAddress addr);
    void setLegoAddress(NimBLEAddress addr);

    // Control settings
    ControlSettings getControlSettings();
    void setControlSettings(ControlSettings settings);

private:
    Preferences prefs;
    static constexpr uint32_t SETTINGS_VERSION = 1;
};
```

### 7. Display Module (Optional - `display.cpp/h`)
**Responsibilities:**
- OLED display management (if added later)
- Status information display
- Simple UI for settings

**Display Options:**
- Serial Monitor (minimal - Phase 1)
- OLED SSD1306 128x64 (optional - Phase 4)
- Web interface via WiFi (advanced option)

**Status Information:**
```cpp
class Display {
public:
    void init();
    void showStatus(const char* message);
    void showConnectionStatus(bool xboxConnected, bool legoConnected);
    void showControlState(int8_t speed, int8_t steering, LegoLightMode lights);
    void showBatteryLevel(uint8_t level);
    void showError(const char* error);

private:
    // Serial or OLED implementation
};
```

**Simple Serial Output (MVP):**
```
Xbox-Lego RC Bridge
-------------------
Xbox: Connected (Battery: 85%)
Lego: Connected
Speed: 45%  Steering: -20%  Lights: ON
Max Speed Limit: 75%
```

## State Machine

### Application States
```c
typedef enum {
    AppStateInit,           // Initializing
    AppStateMainMenu,       // Main menu
    AppStateScanning,       // Scanning for devices
    AppStateConnecting,     // Connecting to devices
    AppStatePairing,        // Pairing with devices
    AppStateCalibrating,    // Calibrating steering
    AppStateActive,         // Active bridging
    AppStateSettings,       // Settings menu
    AppStateError           // Error state
} AppState;
```

### State Transitions
```
Init → MainMenu
MainMenu → Scanning (user selects scan)
MainMenu → Settings (user selects settings)
MainMenu → Active (if already paired)
Scanning → Connecting (devices selected)
Connecting → Pairing (connections established)
Pairing → Calibrating (pairing successful)
Calibrating → Active (calibration complete)
Active → MainMenu (user exits)
Settings → MainMenu (user saves/cancels)
Any → Error (on failure)
Error → MainMenu (user acknowledges)
```

## Threading Model

### ESP32 FreeRTOS Tasks

#### Task 1: Main Loop (Arduino loop)
- Control update logic (20-50 Hz)
- Input sampling from Xbox controller
- Command transmission to Lego hub
- Status updates

#### Task 2: BLE Event Handler (NimBLE task)
- BLE stack events (handled by NimBLE internally)
- GATT callbacks
- Connection events
- Notification handling

#### Task 3: Display Update (optional)
- Periodic display updates (5-10 Hz)
- Battery monitoring
- Status messages

## Data Flow

```
1. Xbox Controller sends HID report
   ↓
2. BLE stack receives notification
   ↓
3. xbox_controller_parse_report() extracts inputs
   ↓
4. control_mapper_map() applies mapping & limits
   ↓
5. lego_hub_send_control() formats command
   ↓
6. BLE stack sends GATT write
   ↓
7. Lego Hub executes command
```

## Memory Management

### ESP32-S3 Resources
- **Total RAM**: 512KB SRAM
- **Flash**: 8MB
- **BLE Stack**: ~39KB for 3 connections (default)

### Heap Allocation Strategy
- NimBLE manages BLE memory automatically
- Use stack for temporary data
- Minimize dynamic allocation in control loop
- Preferences library handles NVS storage

### Estimated Memory Usage
- Application code: ~20-50KB
- BLE stack (NimBLE): ~39KB RAM
- Application objects: ~5KB
- Buffers: ~2KB
- **Total: <100KB** (plenty of headroom with 512KB available)

## Error Handling

### Error Categories
1. **BLE Connection Errors**
   - Timeout
   - Device not found
   - Connection lost
   - Pairing failed

2. **Protocol Errors**
   - Invalid data
   - Unsupported service
   - Characteristic not found

3. **Application Errors**
   - Settings corruption
   - Memory allocation failure

### Error Recovery
- Automatic reconnection attempts (3x with backoff)
- Graceful degradation (continue with one device)
- User notification with clear messages
- Emergency stop on critical failure

## Security Considerations

### BLE Security
- Security Mode 1 Level 2 (unauthenticated encryption) for Lego Hub
- HID over GATT security for Xbox controller
- No sensitive data stored

### Safety Features
- Emergency stop command
- Speed limiting enforcement
- Connection watchdog (auto-stop if disconnected)
- Input validation

## Performance Requirements

### Latency
- Target: <50ms end-to-end latency
- Xbox input sampling: 100Hz (10ms)
- Control update rate: 20-50Hz (20-50ms)
- UI update rate: 10Hz (100ms)

### Power Consumption
- Optimize BLE connection intervals
- Minimize screen updates
- Consider deep sleep when inactive

## File Structure

```
esp32-xbox-lego-rc/
├── platformio.ini                 # PlatformIO configuration
├── src/
│   ├── main.cpp                   # Main application entry
│   ├── ble_manager.cpp            # BLE connection management
│   ├── ble_manager.h
│   ├── xbox_controller.cpp        # Xbox HID over GATT
│   ├── xbox_controller.h
│   ├── lego_hub.cpp               # Lego protocol
│   ├── lego_hub.h
│   ├── control_mapper.cpp         # Input mapping
│   ├── control_mapper.h
│   ├── settings.cpp               # Preferences storage
│   ├── settings.h
│   ├── display.cpp                # Optional display (Phase 4)
│   └── display.h
├── lib/                           # Custom libraries (if any)
├── include/
│   └── config.h                   # Configuration constants
├── docs/
│   ├── ARCHITECTURE.md            # This file
│   └── ESP32_FEASIBILITY.md       # Feasibility analysis
└── README.md                      # Project documentation
```

## Development Phases

### Phase 1: Proof of Concept (1-2 days)
- [x] Architecture design
- [x] Feasibility analysis
- [x] PlatformIO project setup
- [ ] Test Xbox controller connection
- [ ] Test Lego hub connection (Legoino or direct)
- [ ] Verify basic BLE communication

### Phase 2: Core Implementation (1 week)
- [ ] Implement Xbox input reading
- [ ] Implement Lego control commands
- [ ] Basic control mapping (no persistence yet)
- [ ] Serial monitor status output
- [ ] Test end-to-end control flow

### Phase 3: Features (1 week)
- [ ] Add speed limiting
- [ ] Add dead zones
- [ ] Implement light controls
- [ ] Add persistent pairing (Preferences)
- [ ] Battery level monitoring
- [ ] Emergency stop functionality

### Phase 4: Polish (optional)
- [ ] Add OLED display support
- [ ] Settings configuration UI
- [ ] Multiple control profiles
- [ ] Web interface for configuration
- [ ] OTA firmware updates
- [ ] Advanced control features

## Open Questions / Future Enhancements

1. **Legoino Compatibility**: Need to verify if Legoino library supports the new Technic Move Hub 88019. If not, implement direct BLE protocol.

2. **Battery Status for Lego Hub**: Protocol documentation doesn't mention battery queries. May need reverse engineering or Hub doesn't support it.

3. **Multiple Control Profiles**: Allow user to create/save different control mappings for different scenarios.

4. **Telemetry Recording**: Log control inputs to SD card or SPIFFS for analysis.

5. **Multiple Car Support**: ESP32 supports up to 9 BLE connections - could control multiple hubs simultaneously.

6. **Advanced Controls**: Drift mode, speed curves, acceleration limiting, haptic feedback (if controller supports it).

7. **Web Configuration**: Create WiFi AP with web interface for settings configuration.

8. **OTA Updates**: Support over-the-air firmware updates for easy deployment.
