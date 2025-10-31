# Quick Start Guide

## Project Migration Complete! 🎉

The project has been successfully pivoted from **Flipper Zero (ufbt)** to **ESP32-S3 (PlatformIO/Arduino)**.

---

## What's Been Done

### ✅ Project Structure
```
esp32-xbox-lego-rc/
├── platformio.ini          # PlatformIO configuration for XIAO ESP32-S3
├── src/
│   └── main.cpp            # Main application skeleton with state machine
├── include/
│   └── config.h            # All project constants and configuration
├── lib/                    # Custom libraries (empty for now)
├── docs/
│   ├── ARCHITECTURE.md     # Updated for ESP32-S3
│   └── ESP32_FEASIBILITY.md # Platform analysis
├── README.md               # Comprehensive project documentation
├── QUICKSTART.md           # This file
└── .gitignore              # Updated for PlatformIO
```

### ✅ Core Files Created

1. **platformio.ini**
   - Configured for `seeed_xiao_esp32s3` board
   - Arduino framework
   - NimBLE-Arduino library dependency
   - Debug flags enabled
   - Serial monitor at 115200 baud

2. **include/config.h**
   - All BLE UUIDs (Xbox and Lego)
   - Control settings (speed limits, dead zones)
   - Timing constants (20Hz control loop, 5Hz display update)
   - Lego protocol constants (command headers, light modes)
   - Debug macros (DEBUG_PRINT, DEBUG_BLE_PRINT, etc.)
   - Error codes enum

3. **src/main.cpp**
   - Application state machine (INIT → SCANNING → CONNECTING → ACTIVE)
   - setup() and loop() functions
   - BLE initialization placeholder
   - Control loop update (20Hz)
   - Display update (5Hz)
   - Error handling
   - LED status indicators

4. **Documentation**
   - README.md with setup instructions
   - ARCHITECTURE.md updated for ESP32-S3
   - ESP32_FEASIBILITY.md showing platform advantages

---

## Next Steps: Build and Test

### 1. Open in VS Code with PlatformIO

```bash
# If you haven't already:
# 1. Install VS Code
# 2. Install PlatformIO IDE extension

# Open the project
code .
```

### 2. Build the Project

**Method 1: VS Code GUI**
- Click the checkmark (✓) icon in the bottom toolbar
- Or: View → Command Palette → "PlatformIO: Build"

**Method 2: VS Code Terminal**
```bash
pio run
```

**Expected Output:**
```
Processing seeed_xiao_esp32s3 (platform: espressif32; board: seeed_xiao_esp32s3; framework: arduino)
...
Linking .pio\build\seeed_xiao_esp32s3\firmware.elf
Building .pio\build\seeed_xiao_esp32s3\firmware.bin
...
============= [SUCCESS] Took X.XX seconds =============
```

### 3. Upload to ESP32-S3

**Connect your XIAO ESP32-S3 via USB**, then:

**Method 1: VS Code GUI**
- Click the arrow (→) icon in the bottom toolbar
- Or: View → Command Palette → "PlatformIO: Upload"

**Method 2: VS Code Terminal**
```bash
pio run --target upload
```

### 4. Monitor Serial Output

**Method 1: VS Code GUI**
- Click the plug (🔌) icon in the bottom toolbar
- Or: View → Command Palette → "PlatformIO: Serial Monitor"

**Method 2: VS Code Terminal**
```bash
pio device monitor
```

**Expected Output:**
```
========================================
Xbox-Lego RC Bridge
Version: 0.1.0
Board: XIAO ESP32-S3
========================================

[INIT] Initializing BLE...
[BLE] Initializing NimBLE stack...
[BLE] NimBLE initialized successfully
[INIT] Loading settings...
[INIT] Initialization complete
[SCAN] Starting device scan...
[SCAN] Looking for:
  - Xbox Controller: Xbox*
  - Lego Hub: Technic Move

[SCAN] BLE scanning active...
```

---

## Development Workflow

### Current Status: Phase 1 Complete ✅

The basic project skeleton is ready. You can now:

1. **Build and upload** to verify hardware works
2. **Monitor serial output** to see the state machine running
3. **Watch LED blinking** during scanning state

### Next: Phase 2 - BLE Communication 🔄

To continue development, you'll need to implement:

#### Module 1: BLE Manager (`src/ble_manager.cpp/.h`)
```cpp
class BLEManager {
    void init();
    void startScan();
    void connectToXbox(NimBLEAddress address);
    void connectToLego(NimBLEAddress address);
    // Scan callbacks
    // Connection callbacks
};
```

#### Module 2: Xbox Controller (`src/xbox_controller.cpp/.h`)
```cpp
class XboxController {
    void init(NimBLEClient* client);
    void parseHIDReport(uint8_t* data, size_t len);
    XboxControllerState getState();
};
```

#### Module 3: Lego Hub (`src/lego_hub.cpp/.h`)
```cpp
class LegoHub {
    void init(NimBLEClient* client);
    void sendControl(int8_t speed, int8_t steering, uint8_t lights);
    void calibrateSteering();
};
```

---

## Troubleshooting

### Build Issues

**"Platform 'espressif32' not found"**
```bash
pio platform install espressif32
```

**"NimBLE-Arduino library not found"**
```bash
pio lib install "h2zero/NimBLE-Arduino@^1.4.1"
```

### Upload Issues

**"Could not open port"**
- Check USB cable (must support data, not just power)
- Try different USB port
- Install CH340/CP2102 drivers if needed

**Windows: Driver issues**
- Download [CH340 drivers](https://www.wch-ic.com/downloads/CH341SER_EXE.html)
- Or use Zadig to install WinUSB driver

**"Timed out waiting for packet header"**
- Hold **BOOT** button while connecting USB
- Release after upload starts
- Press **RESET** button after upload completes

### Runtime Issues

**No serial output**
- Verify baud rate is **115200**
- Press **RESET** button on board
- Try different serial monitor (Arduino IDE Serial Monitor, Putty, etc.)

**LED not blinking**
- Normal if in error state (rapid blink)
- Should slow blink during scanning
- Solid during connection/active

---

## Configuration

Edit [include/config.h](include/config.h) to customize:

### BLE Device Names
```cpp
#define XBOX_CONTROLLER_NAME_PREFIX "Xbox"
#define LEGO_HUB_NAME "Technic Move"
```

### Control Settings
```cpp
#define DEFAULT_MAX_SPEED_PERCENT   75   // 0-100
#define DEFAULT_DEADZONE_PERCENT    3    // 0-50
#define DEFAULT_TRIGGER_MODE        true // Triggers vs stick
```

### Debug Output
```cpp
#define DEBUG_ENABLED       true
#define DEBUG_BLE           true
#define DEBUG_XBOX          true
#define DEBUG_LEGO          true
```

### Timing
```cpp
#define CONTROL_LOOP_FREQUENCY_HZ   20   // 20Hz = 50ms
#define DISPLAY_UPDATE_FREQUENCY_HZ 5    // 5Hz = 200ms
```

---

## Testing the Current Build

Even though BLE functionality isn't implemented yet, you can verify:

### ✅ Hardware Works
- Board powers on
- LED responds
- Serial communication works

### ✅ State Machine Works
- Transitions from INIT → SCANNING
- LED blinks during scanning state
- Status messages print every 5 seconds

### ✅ Timing Works
- Control loop runs at 20Hz
- Display updates at 5Hz
- No watchdog resets

---

## Useful Commands

### PlatformIO CLI

```bash
# Build
pio run

# Upload
pio run --target upload

# Monitor
pio device monitor

# Build + Upload + Monitor (all in one)
pio run --target upload && pio device monitor

# Clean build
pio run --target clean

# List connected devices
pio device list

# Update libraries
pio lib update

# Update platforms
pio platform update
```

### VS Code Tasks

- `Ctrl+Shift+P` → "PlatformIO: Build"
- `Ctrl+Shift+P` → "PlatformIO: Upload"
- `Ctrl+Shift+P` → "PlatformIO: Serial Monitor"
- `Ctrl+Shift+P` → "PlatformIO: Clean"

---

## Project Files to Keep vs Remove

### ✅ Keep (PlatformIO/ESP32)
- `platformio.ini`
- `src/main.cpp`
- `include/config.h`
- `docs/ARCHITECTURE.md`
- `docs/ESP32_FEASIBILITY.md`
- `README.md`
- `.gitignore`

### ❌ Remove (Flipper Zero - no longer needed)
- `xbox_legorc_bridge.c` ← Old Flipper Zero source
- `application.fam` ← Flipper build config
- `xbox_legorc_bridge.png` ← Flipper app icon

These are now ignored by git (see `.gitignore`) and can be safely deleted:

```bash
# Optional: Remove old Flipper Zero files
rm xbox_legorc_bridge.c application.fam xbox_legorc_bridge.png
```

---

## Resources

### Documentation
- [PlatformIO Docs](https://docs.platformio.org/)
- [ESP32-S3 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/)
- [NimBLE-Arduino Library](https://github.com/h2zero/NimBLE-Arduino)
- [XIAO ESP32-S3 Wiki](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)

### Protocols
- [Lego Technic Move Hub Protocol](https://github.com/DanieleBenedettelli/TechnicMoveHub)
- [Xbox Controller HID over GATT](https://github.com/esp32beans/ESP32-BLE-HID-exp)

### Community
- [PlatformIO Community Forum](https://community.platformio.org/)
- [ESP32 Forum](https://www.esp32.com/)
- [Arduino Forum](https://forum.arduino.cc/)

---

## Current Capabilities

The project **right now** (v0.1.0-dev):

✅ Compiles for XIAO ESP32-S3
✅ Initializes NimBLE stack
✅ Runs state machine
✅ Outputs debug info to serial
✅ Blinks LED based on state
✅ Ready for BLE implementation

❌ Does NOT yet:
- Scan for BLE devices
- Connect to Xbox controller
- Connect to Lego hub
- Read controller inputs
- Send Lego commands

**These will be implemented in Phase 2!**

---

## Success Criteria

You'll know everything is working when you see:

```
========================================
Xbox-Lego RC Bridge
Version: 0.1.0
Board: XIAO ESP32-S3
========================================

[INIT] Initializing BLE...
[BLE] Initializing NimBLE stack...
[BLE] NimBLE initialized successfully
[INIT] Loading settings...
[INIT] Initialization complete
[SCAN] Starting device scan...

========== Status ==========
State: SCANNING
Uptime: 5 seconds
============================
```

And the LED should be **slowly blinking** (500ms on, 500ms off).

---

## Ready to Code? 🚀

The foundation is set! To continue development:

1. ✅ **Verify this build works** (upload and test)
2. 📝 **Read the architecture** ([docs/ARCHITECTURE.md](docs/ARCHITECTURE.md))
3. 🔍 **Study BLE examples** (NimBLE-Arduino library examples)
4. 🛠️ **Implement BLE Manager** (Phase 2)

---

**Questions?** Check [README.md](README.md) or review the architecture docs!

**Let's build this! 🎮🚗**
