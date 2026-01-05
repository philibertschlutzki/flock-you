# Flock You: WiFi/BLE Unknown-Device Detection

<img src="flock.png" alt="Flock You" width="300px">

**Unknown device detection firmware for the LILYGO T-Display-S3 ESP32-S3 board**

## Overview

Flock You is a WiFi and BLE scanning system that monitors the environment and **reports only unknown devices** (i.e., devices not in a compile-time blacklist).

The firmware target is the **LILYGO T-Display-S3 (ESP32-S3 + 1.9\" ST7789 LCD)**.

## Features

### Unknown-only Detection (Blacklist)
- **WiFi Promiscuous Mode**: Captures probe requests and beacon frames
- **Bluetooth Low Energy (BLE) Active Scanning**: Monitors BLE advertisements
- **Blacklist-based Filtering**: Known devices are filtered out; only unknowns are reported
- **Matching without Wildcards/Patterns**:
  - SSID: exact, **case-sensitive** match
  - MAC: 6-byte comparison
  - OUI: 3-byte prefix (manufacturer prefix)
- **Session Filter (RAM-only)**: Runtime blacklist extension (e.g., for future WebGUI/API), resets on reboot

### Display UI (LILYGO T-Display-S3)
- **Status screen**: WiFi channel, BLE status, uptime
- **Alert screen**: Detection alerts (WiFi/BLE, RSSI, short text)
- **Backlight control**: On/Off, PWM brightness
- **Hardware diagnostics**: Full self-test

### Output
- **JSON over Serial**: For logging/post-processing
- **Optional Web Dashboard**: In `api/` folder (consumes serial JSON)

## Hardware Requirements

### LILYGO T-Display-S3
- **Microcontroller**: ESP32-S3 with PSRAM
- **Display**: 1.9\" ST7789 TFT LCD (170x320 pixels)
- **Wireless**: Dual WiFi/BLE scanning capabilities
- **Buttons**: 2x buttons (A and B) for interaction
- **Backlight**: PWM-controllable backlight
- **Connectivity**: USB-C for programming and power
- **No Audio**: Buzzer not present (display-based alerts)

**Where to buy:**
- AliExpress, Amazon, Tindie
- Price: approx. 15-25 EUR/USD

## Installation

### Prerequisites
- PlatformIO IDE or PlatformIO Core
- Python 3.7+ (for blacklist generator)
- USB-C cable for programming
- LILYGO T-Display-S3 board

### Quick Start
1. **Clone the repository**:
   ```bash
   git clone https://github.com/philibertschlutzki/flock-you.git
   cd flock-you
   ```

2. **Install PlatformIO** (if not already installed):
   ```bash
   pip install -U platformio
   ```

3. **Generate blacklist** (required for build):
   ```bash
   python3 tools/generate_blacklist_from_csv.py
   ```

4. **Compile and flash firmware**:
   ```bash
   pio run --target upload
   ```

5. **Open serial monitor** (optional):
   ```bash
   pio device monitor
   ```

If you modify `datasets/`, you must regenerate the blacklist.

## Detection Principle

### Core Logic
- **Known** (in Blacklist/Session Filter) → discarded/ignored
- **Unknown** (not in Blacklist/Session Filter) → reported as detection

### Detection Types
- WiFi: e.g., `probe_request_unknown`, `beacon_unknown`
- BLE: `unknown`

Details (including matching rules and Session Filter API) are described in the filter documentation:
📘 **[Blacklist, Filter & Matching (German)](docs/FILTERS_AND_MATCHING_DE.md)**

## Project Structure (Excerpt)

```
flock-you/
├── platformio.ini
├── README.md
├── datasets/                       # Input for blacklist generator (CSV)
├── tools/
│   └── generate_blacklist_from_csv.py
├── include/
│   ├── config.h
│   ├── tdisplay_s3_tft_espi_setup.h
│   ├── diagnostics.h
│   ├── ui_display.h
│   ├── wifi_sniffer.h
│   ├── ble_scanner.h
│   ├── blacklist.h
│   ├── session_filter.h
│   ├── known_blacklist_generated.h  # generated
│   └── output.h
├── src/
│   ├── main.cpp
│   ├── diagnostics.cpp
│   ├── ui_display.cpp
│   ├── wifi_sniffer.cpp
│   ├── ble_scanner.cpp
│   ├── blacklist.cpp
│   ├── session_filter.cpp
│   ├── known_blacklist_generated.cpp # generated
│   └── output.cpp
└── docs/
    ├── FILTERS_AND_MATCHING_DE.md
    ├── FLASHING_LILYGO_TDISPLAY_S3_DE.md
    ├── HARDWARE_DIAGNOSTICS_DE.md
    └── ROADMAP.md
```

## Contributing

We welcome contributions!

- 🐛 Bug Reports: https://github.com/philibertschlutzki/flock-you/issues
- 💡 Feature Requests: https://github.com/philibertschlutzki/flock-you/discussions
- 🔧 Pull Requests: Code, documentation

## License

[Insert License - e.g., MIT, GPL, etc.]

## ⚠️ Disclaimer

This project is for educational and research purposes only. The use of WiFi Promiscuous Mode and BLE Scanning is subject to local laws. Users are responsible for compliance with applicable laws.
