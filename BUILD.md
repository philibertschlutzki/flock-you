# Build Instructions for Flock You

## Prerequisites

### Required Software
- PlatformIO Core (latest version)
- Python 3.7+ (for PlatformIO)

### Platform Dependencies
- espressif32 platform
- ESP32-S3 toolchain

### Library Dependencies (auto-installed by PlatformIO)
- NimBLE-Arduino ^1.4.0
- ArduinoJson ^6.21.0
- TFT_eSPI ^2.5.43

## Build Commands

### Standard Build
```bash
# Build firmware
pio run -e lilygo_tdisplay_s3

# Build and upload
pio run -e lilygo_tdisplay_s3 -t upload

# Monitor serial output
pio device monitor
```

### Clean Build
```bash
# Clean build directory
pio run -e lilygo_tdisplay_s3 -t clean

# Full clean and rebuild
pio run -e lilygo_tdisplay_s3 -t clean && pio run -e lilygo_tdisplay_s3
```

## Expected Build Output

### Successful Build
```
Processing lilygo_tdisplay_s3 (platform: espressif32; board: esp32-s3-devkitc-1; framework: arduino)
------------------------------------------------------------------------------------
Compiling .pio/build/lilygo_tdisplay_s3/src/ble_scanner.cpp.o
Compiling .pio/build/lilygo_tdisplay_s3/src/diagnostics.cpp.o
Compiling .pio/build/lilygo_tdisplay_s3/src/main.cpp.o
Compiling .pio/build/lilygo_tdisplay_s3/src/output.cpp.o
Compiling .pio/build/lilygo_tdisplay_s3/src/state.cpp.o
Compiling .pio/build/lilygo_tdisplay_s3/src/ui_display.cpp.o
Compiling .pio/build/lilygo_tdisplay_s3/src/wifi_sniffer.cpp.o
Linking .pio/build/lilygo_tdisplay_s3/firmware.elf
Building .pio/build/lilygo_tdisplay_s3/firmware.bin
===== [SUCCESS] =====
```

### Output Location
- Compiled firmware: `.pio/build/lilygo_tdisplay_s3/firmware.bin`
- ELF file: `.pio/build/lilygo_tdisplay_s3/firmware.elf`

## Compile-Time Configuration

### Feature Flags (platformio.ini)
```ini
-DFLOCKYOU_NO_BUZZER=1        # Disable buzzer (T-Display-S3 has no buzzer)
-DFLOCKYOU_HAS_DISPLAY=1      # Enable TFT display support
```

### TFT_eSPI Configuration
The TFT_eSPI library is automatically configured via:
```ini
-DUSER_SETUP_LOADED=1
-include include/tdisplay_s3_tft_espi_setup.h
```

This configuration is specific to LILYGO T-Display-S3:
- ST7789 driver (1.9" 320x170 display)
- i8080 8-bit parallel interface
- Backlight on GPIO 38 (TFT_BL)

## Module Compilation

All source files are compiled independently:

| Module | Purpose | Dependencies |
|--------|---------|--------------|
| `main.cpp` | Orchestration | config.h, state.h, all module headers |
| `state.cpp` | Global state | state.h |
| `wifi_sniffer.cpp` | WiFi scanning | state.h, output.h, detection_patterns.h |
| `ble_scanner.cpp` | BLE scanning | state.h, output.h, detection_patterns.h |
| `output.cpp` | JSON/Display output | state.h, wifi_sniffer.h, ui_display.h |
| `ui_display.cpp` | TFT rendering | state.h, wifi_sniffer.h, TFT_eSPI |
| `diagnostics.cpp` | Hardware test | config.h, TFT_eSPI |

## Common Build Issues & Solutions

### Issue: Platform not installed
```
HTTPClientError: Failed to install espressif32
```
**Solution:**
```bash
pio pkg install --platform espressif32
```

### Issue: Library not found
```
Error: NimBLE-Arduino not found
```
**Solution:**
```bash
pio lib install "h2zero/NimBLE-Arduino@^1.4.0"
pio lib install "bblanchon/ArduinoJson@^6.21.0"
pio lib install "bodmer/TFT_eSPI@^2.5.43"
```

### Issue: Duplicate symbol definitions
```
multiple definition of `triggered'
```
**This should NOT occur after refactoring. If it does:**
- Verify `state.cpp` is the only file with `bool triggered = false;`
- Check that all other files only have `#include "state.h"`
- Run validation: `/tmp/validate_refactoring.sh`

### Issue: TFT_BACKLIGHT_PIN not defined
```
error: 'TFT_BACKLIGHT_PIN' was not declared
```
**This should NOT occur after refactoring. If it does:**
- Use `TFT_BL` instead (from tdisplay_s3_tft_espi_setup.h)
- Verify config.h does NOT define TFT_BACKLIGHT_PIN

## Verification Steps

After building, verify:

1. **Firmware size:** Should be ~1-2 MB
```bash
ls -lh .pio/build/lilygo_tdisplay_s3/firmware.bin
```

2. **No duplicate symbols:** Check build log for "multiple definition"
```bash
pio run -e lilygo_tdisplay_s3 2>&1 | grep -i "multiple definition"
# Should return nothing
```

3. **All modules compiled:**
```bash
ls .pio/build/lilygo_tdisplay_s3/src/*.o
# Should list 7 .o files (one per .cpp file)
```

## Flash to Device

### USB Connection
1. Connect LILYGO T-Display-S3 via USB-C
2. Device should appear as `/dev/ttyACM0` or similar

### Upload
```bash
pio run -e lilygo_tdisplay_s3 -t upload
```

### Monitor Serial Output
```bash
pio device monitor --baud 115200
```

Expected initial output:
```
Starting Flock You Detection System...

WiFi promiscuous mode enabled on channel 1
Monitoring probe requests and beacons...
Initializing BLE scanner...
BLE scanner initialized
System ready - hunting for Flock Safety devices...
```

## Hardware Diagnostics Mode

To enter diagnostics mode:
1. Hold Button A (GPIO 0)
2. Press Reset or power on
3. Keep holding Button A for 3 seconds
4. Diagnostics will run automatically

Tests performed:
- Display RGB test
- Backlight PWM test
- Button input test
- WiFi promiscuous mode test
- BLE scanning test
- PSRAM/Heap memory test

## Memory Usage

Typical flash usage:
- Sketch: ~900 KB
- Total: ~1.2 MB (with bootloader, partitions)

Typical RAM usage:
- Global variables: <10 KB
- Stack: ~8 KB
- Heap: ~250 KB free (varies during BLE/WiFi operations)
- PSRAM: ~8 MB available (OPI PSRAM on T-Display-S3)

## Development Tips

### Fast iteration
```bash
# Watch for changes and rebuild
pio run -e lilygo_tdisplay_s3 -t upload && pio device monitor
```

### Debug output
Add to platformio.ini:
```ini
build_flags = 
    ...existing flags...
    -DCORE_DEBUG_LEVEL=5  # Verbose ESP32 core debug
```

### Code size optimization
For release builds, add:
```ini
build_flags = 
    ...existing flags...
    -Os  # Optimize for size
```

## Support

For build issues:
1. Check this document
2. Run validation script: `/tmp/validate_refactoring.sh`
3. Check REFACTORING_SUMMARY.md
4. Verify PlatformIO version: `pio --version` (should be 6.1+)
