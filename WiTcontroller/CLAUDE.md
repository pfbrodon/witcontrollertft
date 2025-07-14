# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

WiTcontroller is an ESP32-based handheld controller for model trains that communicates with WiThrottle servers (JMRI, DCC-EX, etc.) using the WiThrottle protocol. The project provides a simple DIY throttle with minimal components.

## Development Commands

### Build and Upload
```bash
# Build the project
pio run

# Upload to ESP32 (WEMOS LOLIN32 Lite)
pio run --target upload

# Monitor serial output
pio device monitor

# Build and upload in one command
pio run --target upload && pio device monitor
```

### Library Management
```bash
# Install dependencies (automatically handled by platformio.ini)
pio lib install

# Update libraries
pio lib update
```

## Architecture and Code Structure

### Core Files
- `WiTcontroller.ino` - Main Arduino sketch file containing setup() and loop()
- `platformio.ini` - PlatformIO configuration with dependencies and build settings
- `WiTcontroller.h` - Main header with function declarations and constants

### Configuration Files (Must be created from examples)
- `config_network.h` - WiFi network configurations (copy from example)
- `config_buttons.h` - Button mappings and hardware settings (copy from example)

### Library Dependencies
- `WiThrottleProtocol` - Communication with WiThrottle servers
- `U8g2` - OLED display driver
- `Keypad` - 3x4/4x4 keypad input handling
- `AiEsp32RotaryEncoder` - Rotary encoder for throttle control
- `Pangodream_18650_CL` - Battery monitoring (included locally)

### Hardware Configuration
- ESP32 WEMOS LOLIN32 Lite as main controller
- 0.96" OLED display (SSD1306) via I2C
- 3x4 or 4x4 keypad for input
- KY-040 rotary encoder for throttle control
- Optional: Additional buttons, battery monitoring, power switch

### Key Architecture Patterns
- **State Management**: Uses multiple throttles (up to 6) with independent speed/direction
- **Menu System**: Hierarchical command structure accessed via * key
- **Configuration**: Extensive compile-time configuration via header files
- **Network**: Auto-discovery of WiThrottle servers via mDNS
- **Display**: Modular display system with abstraction layer for different screen types
  - `DisplayInterface` - Abstract interface for all display types
  - `DisplayManager` - High-level display management with generic functions
  - `DisplayOLED_U8G2` - U8g2 OLED implementation
  - `DisplayFactory` - Factory pattern for creating display instances

### Pin Assignments (Default)
- I2C (OLED): SDA=23, SCL=22
- Rotary Encoder: DT=12, SW=13, CLK=14
- Keypad: Columns on pins 0,2,4,(33 for 4x4), Rows on pins 19,18,17,16
- Additional buttons: pins 5,15,25,26,27,32,33,34,35,36,39 (with restrictions)

### Translation Support
- German translations in `language_deutsch.h`
- Italian translations in `language_italiano.h`
- Translation system via preprocessor defines

## Important Notes

### Configuration Setup
Before building, you must:
1. Copy `config_network_example.h` to `config_network.h`
2. Copy `config_buttons_example.h` to `config_buttons.h`
3. Edit these files for your specific setup

### Hardware Variations
The codebase supports multiple hardware configurations through compile-time defines:
- Different OLED sizes (0.96", 1.3", 2.4")
- 3x4 vs 4x4 keypads
- Potentiometer vs rotary encoder for throttle
- Optional battery monitoring
- Up to 11 additional buttons

### Library Versions
Specific library versions are locked in platformio.ini to ensure compatibility. Check README.md for version compatibility matrix.

### Development Workflow
1. Make hardware configuration changes in `config_*.h` files
2. Use PlatformIO for building and uploading
3. Monitor serial output for debugging (115200 baud)
4. Debug level can be set via `WITCONTROLLER_DEBUG` define

## Display System Architecture

### Modular Display System
The project now includes a modular display system that abstracts hardware-specific display code:

**Core Files:**
- `display_interface.h` - Abstract interface for all display types
- `display_manager.h/.cpp` - High-level display management
- `display_oled_u8g2.h/.cpp` - U8G2 OLED implementation
- `display_factory.cpp` - Factory for creating display instances
- `display_integration_example.h` - Migration guide and examples

### Display Integration
To use the new modular display system:

1. **Replace display function calls:**
   - `writeOledSpeed()` → `displayManager->showThrottleScreen()`
   - `writeOledMenu()` → `displayManager->showMenu()`
   - `writeOledRoster()` → `displayManager->showRosterList()`

2. **Initialize in setup():**
   ```cpp
   displayInterface = DisplayFactory::createOLEDDisplay(&u8g2);
   displayManager = new DisplayManager(displayInterface);
   displayManager->begin();
   ```

3. **Benefits:**
   - Easy to add support for TFT, LCD, or other display types
   - Cleaner separation of display logic from application logic
   - More maintainable and testable code
   - Generic function names independent of hardware

### Adding New Display Types
To add support for new display hardware:
1. Create new implementation class inheriting from `DisplayInterface`
2. Implement all virtual methods for the specific hardware
3. Add factory method in `DisplayFactory`
4. No changes needed in application code