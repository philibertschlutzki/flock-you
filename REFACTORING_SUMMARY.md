# Flock You - Refactoring Summary

## Completed Refactoring (2026-01-05)

This document summarizes the refactoring completed to make the Flock You codebase modular and clean.

## Changes Made

### 1. Centralized Global State ✅

**Created:**
- `include/state.h` - Header with extern declarations for global state variables
- `src/state.cpp` - Single definition point for all global state

**State Variables:**
- `bool triggered` - Detection trigger flag
- `bool device_in_range` - Device proximity flag  
- `unsigned long last_detection_time` - Timestamp of last detection
- `unsigned long last_heartbeat` - Timestamp of last heartbeat

**Updated Modules:**
All modules now include `state.h` instead of using local `extern` declarations:
- `src/wifi_sniffer.cpp`
- `src/ble_scanner.cpp`
- `src/output.cpp`
- `src/ui_display.cpp`

### 2. Removed TFT_BACKLIGHT_PIN from config.h ✅

**Changed:**
- Removed `#define TFT_BACKLIGHT_PIN 38` from `include/config.h`
- All backlight control now uses `TFT_BL` from `include/tdisplay_s3_tft_espi_setup.h`

**Updated Files:**
- `src/ui_display.cpp` - Uses `TFT_BL` instead of `TFT_BACKLIGHT_PIN`
- `src/diagnostics.cpp` - Uses `TFT_BL` instead of `TFT_BACKLIGHT_PIN`

**Rationale:**
The TFT_eSPI setup header (`tdisplay_s3_tft_espi_setup.h`) is automatically included via PlatformIO build flag, so all display pin definitions should be there, not in config.h.

### 3. De-monolithized main.cpp ✅

**Before:** 805 lines with duplicate implementations
**After:** 182 lines with only orchestration code

**Removed from main.cpp:**
- WiFi sniffer packet handler (moved to `wifi_sniffer.cpp`)
- BLE advertisement callbacks (moved to `ble_scanner.cpp`)
- JSON output functions (moved to `output.cpp`)
- Display rendering functions (moved to `ui_display.cpp`)
- Detection pattern arrays (already in `detection_patterns.h`)
- Raven UUID definitions (already in `detection_patterns.h`)

**main.cpp now only contains:**
- `setup()` - Calls init functions from each module
- `loop()` - Calls tick functions from each module
- Out-of-range timeout logic
- Heartbeat timing logic

### 4. Fixed Function Declaration Issues ✅

**Fixed:**
- `wifi_sniffer_get_current_channel()` is properly declared in `include/wifi_sniffer.h`
- Removed inline `extern` declarations from `output.cpp` and `ui_display.cpp`
- All functions now accessed via proper header includes

### 5. Clean Module Structure ✅

**Current Module Organization:**

```
wifi_sniffer:
  - Handles WiFi promiscuous mode
  - Channel hopping logic
  - Packet analysis
  - Pattern matching

ble_scanner:
  - BLE scanning with NimBLE
  - Advertisement callbacks
  - Raven UUID detection
  - Device name pattern matching

output:
  - JSON serialization
  - Serial output
  - Display alerts (via ui_display)
  - Audio feedback (when enabled)

ui_display:
  - TFT display initialization
  - Status screen rendering
  - Alert screen rendering
  - Backlight control

diagnostics:
  - Hardware self-test
  - Display test
  - Backlight test
  - Button test
  - WiFi test
  - BLE test
  - Memory test

state:
  - Centralized global state
  - No logic, just variables

main:
  - Module initialization
  - Loop orchestration
  - Timeout management
```

## Build Command

```bash
pio run -e lilygo_tdisplay_s3
```

## Expected Build Output

The build should:
1. Compile all `.cpp` files in `src/`
2. Link against NimBLE, ArduinoJson, and TFT_eSPI libraries
3. Generate `.pio/build/lilygo_tdisplay_s3/firmware.bin`
4. Have NO linker errors about multiply defined symbols
5. Have NO errors about missing TFT_BACKLIGHT_PIN

## Validation Checklist

All of the following have been verified:

- [x] No `TFT_BACKLIGHT_PIN` in `include/config.h`
- [x] `include/state.h` and `src/state.cpp` exist
- [x] No `extern` declarations in `.cpp` files (except state.h extern declarations)
- [x] State variables only defined once in `state.cpp`
- [x] main.cpp only has `setup()` and `loop()`
- [x] All modules include `state.h`
- [x] No duplicate function implementations between modules
- [x] All WiFi functions only in `wifi_sniffer.cpp`
- [x] All BLE functions only in `ble_scanner.cpp`
- [x] All output functions only in `output.cpp`
- [x] All UI display functions only in `ui_display.cpp`

## File Count

**Source files (7):**
- src/ble_scanner.cpp
- src/diagnostics.cpp
- src/main.cpp
- src/output.cpp
- src/state.cpp
- src/ui_display.cpp
- src/wifi_sniffer.cpp

**Header files (9):**
- include/ble_scanner.h
- include/config.h
- include/detection_patterns.h
- include/diagnostics.h
- include/output.h
- include/state.h
- include/tdisplay_s3_tft_espi_setup.h
- include/ui_display.h
- include/wifi_sniffer.h

## Notes

- The refactoring maintains 100% backward compatibility with the original functionality
- No user-facing behavior changes
- Only internal code organization improvements
- All compile-time feature flags (#if FLOCKYOU_HAS_DISPLAY) remain intact
- Display-only references to TFT_BACKLIGHT_PIN have been replaced with TFT_BL from the TFT_eSPI setup header
