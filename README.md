# Xbox-Lego RC Bridge

**A Bluetooth Low Energy bridge between an Xbox One controller and Lego Technic Move Hub**

Control your Lego Technic 42176 Porsche GT4 e-performance RC car with an Xbox One controller via BLE!

---

## Platform

- **Hardware**: Seeed XIAO ESP32-S3
- **Framework**: Arduino (PlatformIO)
- **Language**: C++

---

## Features

### Current (v0.1.0 - In Development)
- BLE scanning for Xbox controller and Lego hub
- Basic connection management
- Serial debug output

### Planned
- ✅ Xbox controller input reading (HID over GATT)
- ✅ Lego Technic Move Hub control (custom BLE protocol)
- ✅ Control mapping (triggers/sticks → speed/steering)
- ✅ Speed limiting and dead zones
- ✅ Light control modes
- ✅ Persistent device pairing (NVS storage)
- ✅ Battery level monitoring
- ⏳ OLED display support (optional)
- ⏳ Web interface for configuration (optional)
- ⏳ OTA firmware updates (optional)

---

## Hardware Requirements

### Required
1. **XIAO ESP32-S3** - Seeed Studio development board
   - [Product Page](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html)
   - ~$7 USD

2. **Xbox One Controller (Model 1914)** - Bluetooth version
   - Any Xbox One S/X controller with BLE support
   - Check: Controller has plastic around Xbox button (not glossy)

3. **Lego Technic Move Hub (88019)** - From set 42176
   - Comes with Lego Technic 42176 Porsche GT4 e-performance
   - Note: This is the NEW Technic Move Hub (2024), different from older Move Hub

### Optional
- **SSD1306 OLED Display** (128x64) for status information
- **Micro-USB cable** for programming and serial debugging

---

## Software Requirements

### Development Tools
- [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) (VS Code extension)
  - OR [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html) (CLI)
- USB drivers for ESP32-S3

### Libraries (Auto-installed by PlatformIO)
- **NimBLE-Arduino** - BLE stack
- Additional libraries TBD based on implementation approach

---

## Project Structure

```
esp32-xbox-lego-rc/
├── platformio.ini              # PlatformIO configuration
├── src/
│   ├── main.cpp                # Main application
│   ├── ble_manager.cpp         # BLE connection management (TODO)
│   ├── ble_manager.h
│   ├── xbox_controller.cpp     # Xbox HID parsing (TODO)
│   ├── xbox_controller.h
│   ├── lego_hub.cpp            # Lego protocol (TODO)
│   ├── lego_hub.h
│   ├── control_mapper.cpp      # Input mapping (TODO)
│   ├── control_mapper.h
│   ├── settings.cpp            # NVS storage (TODO)
│   └── settings.h
├── include/
│   └── config.h                # Configuration constants
├── lib/                        # Custom libraries (if any)
├── docs/
│   ├── ARCHITECTURE.md         # System architecture
│   └── ESP32_FEASIBILITY.md    # Platform analysis
└── README.md                   # This file
```

---

## Getting Started

### 1. Clone the Repository

```bash
git clone <your-repo-url>
cd esp32-xbox-lego-rc
```

### 2. Install PlatformIO

If using VS Code:
1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install the [PlatformIO IDE extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)

If using CLI:
```bash
pip install platformio
```

### 3. Open the Project

**VS Code:**
- File → Open Folder → Select `esp32-xbox-lego-rc`
- PlatformIO will automatically detect the project

**CLI:**
```bash
cd esp32-xbox-lego-rc
pio run
```

### 4. Build the Project

**VS Code:**
- Click the checkmark icon (✓) in the PlatformIO toolbar
- OR: Terminal → Run Task → PlatformIO: Build

**CLI:**
```bash
pio run
```

### 5. Upload to ESP32-S3

**VS Code:**
- Connect XIAO ESP32-S3 via USB
- Click the arrow icon (→) in the PlatformIO toolbar
- OR: Terminal → Run Task → PlatformIO: Upload

**CLI:**
```bash
pio run --target upload
```

### 6. Monitor Serial Output

**VS Code:**
- Click the plug icon (🔌) in the PlatformIO toolbar
- OR: Terminal → Run Task → PlatformIO: Monitor

**CLI:**
```bash
pio device monitor
```

---

## Configuration

Edit [include/config.h](include/config.h) to customize:

- **BLE device names** to match your devices
- **Control settings** (max speed, dead zones, etc.)
- **Debug output** levels
- **Pin assignments** (if using OLED display)

---

## Development Phases

### ✅ Phase 1: Project Setup (COMPLETE)
- [x] Create PlatformIO project structure
- [x] Configure for XIAO ESP32-S3
- [x] Add NimBLE library
- [x] Basic skeleton application

### 🔄 Phase 2: BLE Communication (IN PROGRESS)
- [ ] Implement BLE scanning
- [ ] Xbox controller connection
- [ ] Lego hub connection
- [ ] GATT service discovery

### ⏳ Phase 3: Control Implementation
- [ ] Xbox input parsing
- [ ] Lego command transmission
- [ ] Basic control mapping
- [ ] Test end-to-end control

### ⏳ Phase 4: Features
- [ ] Speed limiting
- [ ] Dead zones
- [ ] Light controls
- [ ] Battery monitoring
- [ ] Persistent pairing

### ⏳ Phase 5: Polish
- [ ] OLED display (optional)
- [ ] Web interface (optional)
- [ ] Multiple control profiles
- [ ] OTA updates

---

## Control Mapping

### Default Controls

| Xbox Input | Lego Output | Notes |
|------------|-------------|-------|
| Right Trigger | Forward throttle | 0-100% |
| Left Trigger | Reverse throttle | 0-100% |
| Left Stick X | Steering | -100% to +100% |
| A Button | Toggle lights | On/Off |
| B Button | Cycle light modes | All/Brake/Rear/Off |
| Right Bumper | Brake lights | While held |
| B + X | Emergency stop | Immediate stop |

### Settings (Configurable)
- **Max Speed**: 0-100% (default: 75%)
- **Dead Zone**: 0-50% (default: 3%)
- **Control Mode**: Triggers or Left Stick Y for throttle
- **Invert Steering**: Normal or inverted

---

## Troubleshooting

### Build Errors

**"Platform 'espressif32' not found"**
```bash
pio platform install espressif32
```

**Library dependency errors**
```bash
pio lib install
```

### Upload Errors

**"Could not open port"**
- Check USB cable (must be data cable, not charge-only)
- Install CH340/CP2102 drivers if needed
- Press RESET button on XIAO while uploading

**"Timed out waiting for packet header"**
- Hold BOOT button while connecting USB
- Release after upload starts

### Runtime Issues

**No serial output**
- Check baud rate is set to 115200
- Try pressing RESET button

**Can't find Xbox controller**
- Make sure controller is in pairing mode (hold pairing button)
- Check controller is BLE model (1914, not older models)

**Can't connect to Lego hub**
- Turn hub on and off
- Make sure hub is not connected to official Lego app
- Check battery level in hub

---

## Technical Documentation

- [ARCHITECTURE.md](docs/ARCHITECTURE.md) - System design and module structure
- [ESP32_FEASIBILITY.md](docs/ESP32_FEASIBILITY.md) - Platform analysis and design decisions

---

## References

### Xbox Controller
- [ESP32-BLE-HID-exp Library](https://github.com/esp32beans/ESP32-BLE-HID-exp)
- HID over GATT Specification
- Xbox Wireless Controller Protocol

### Lego Technic Move Hub
- [TechnicMoveHub Protocol](https://github.com/DanieleBenedettelli/TechnicMoveHub)
- [Python Reference Implementation](https://github.com/DanieleBenedettelli/TechnicMoveHub/blob/main/LEGO%20Technic%2042176%20XBOX%20RC.py)
- [Legoino Library](https://github.com/corneliusmunz/legoino) (for Powered UP)

### ESP32-S3
- [XIAO ESP32-S3 Documentation](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
- [NimBLE-Arduino Documentation](https://github.com/h2zero/NimBLE-Arduino)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/en/latest/)

---

## License

[Choose your license - MIT, GPL, etc.]

---

## Contributing

Contributions welcome! Please open an issue or pull request.

---

## Acknowledgments

- [DanieleBenedettelli](https://github.com/DanieleBenedettelli) - Lego Technic Move Hub protocol documentation
- [esp32beans](https://github.com/esp32beans) - Xbox controller BLE library
- [h2zero](https://github.com/h2zero) - NimBLE-Arduino library
- Seeed Studio - XIAO ESP32-S3 hardware

---

## Status

**Current Version**: 0.1.0-dev
**Status**: 🔧 In Development
**Last Updated**: 2025-01-30

---

**Questions or Issues?** Open an issue on GitHub!
