# Flock You: Flock Safety Detection System

<img src="flock.png" alt="Flock You" width="300px">

**Professional surveillance camera detection for the Oui-Spy device available at [colonelpanic.tech](https://colonelpanic.tech)**

## Overview

Flock You is an advanced detection system designed to identify Flock Safety surveillance cameras, Raven gunshot detectors, and similar surveillance devices using multiple detection methodologies.

The main firmware target is the **Seeed XIAO ESP32S3**, but the project also supports a **display-only build** for the **LILYGO T-Display-S3 (ESP32-S3 + 1.9\" ST7789 LCD)**.

## Features

### Multi-Method Detection
- **WiFi Promiscuous Mode**: Captures probe requests and beacon frames
- **Bluetooth Low Energy (BLE) Scanning**: Monitors BLE advertisements
- **MAC Address Filtering**: Detects devices by known MAC prefixes
- **SSID Pattern Matching**: Identifies networks by specific names
- **Device Name Pattern Matching**: Detects BLE devices by advertised names
- **BLE Service UUID Detection**: Identifies Raven gunshot detectors by service UUIDs (NEW)

### Audio Alert System (XIAO / Oui-Spy builds)
- **Boot Sequence**: 2 beeps (low pitch → high pitch) on startup
- **Detection Alert**: 3 fast high-pitch beeps when device detected
- **Heartbeat Pulse**: 2 beeps every 10 seconds while device remains in range
- **Range Monitoring**: Automatic detection of device leaving range

### Display Alert System (LILYGO T-Display-S3 build)
- **Status screen**: Current WiFi channel, BLE scan state, uptime, and device-in-range indicator
- **Alert screen**: Shows last detection (WiFi/BLE/Raven) with RSSI and a short label

### Comprehensive Output
- **JSON Detection Data**: Structured output with timestamps, RSSI, MAC addresses
- **Real-time Web Dashboard**: Live monitoring at `http://localhost:5000`
- **Serial Terminal**: Real-time device output in the web interface
- **Detection History**: Persistent storage and export capabilities (CSV, KML)
- **Device Information**: Full device details including signal strength and threat assessment
- **Detection Method Tracking**: Identifies which detection method triggered the alert

## Hardware Requirements

### Option 1: Oui-Spy Device (Available at colonelpanic.tech)
- **Microcontroller**: Xiao ESP32 S3
- **Wireless**: Dual WiFi/BLE scanning capabilities
- **Audio**: Built-in buzzer system
- **Connectivity**: USB-C for programming and power

### Option 2: Standard Xiao ESP32 S3 Setup
- **Microcontroller**: Xiao ESP32 S3 board
- **Buzzer**: 3V buzzer connected to GPIO3 (D2)
- **Power**: USB-C cable for programming and power

### Wiring for Standard Setup
```
Xiao ESP32 S3    Buzzer
GPIO3 (D2)  ---> Positive (+)
GND         ---> Negative (-)
```

### Option 3: LILYGO T-Display-S3 (Display-only, no buzzer)
- **Microcontroller**: ESP32-S3
- **Display**: 1.9\" ST7789 LCD (used for status + alerts)
- **Audio**: Not required (buzzer disabled in this build)
- **Power**: USB-C

## Installation

### Prerequisites
- PlatformIO IDE or PlatformIO Core
- Python 3.8+ (for web interface)
- USB-C cable for programming
- Oui-Spy device from [colonelpanic.tech](https://colonelpanic.tech)

### Setup Instructions
1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd flock-you
   ```

2. **Flash the firmware** (XIAO default env):
   ```bash
   pio run --target upload
   ```

3. **Flash the firmware (LILYGO T-Display-S3 display-only env)**:
   ```bash
   pio run -e lilygo_t_display_s3_display_only --target upload
   ```

4. **Set up the web interface**:
   ```bash
   cd api
   python3 -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   pip install -r requirements.txt
   ```

5. **Start the web server**:
   ```bash
   python flockyou.py
   ```

6. **Access the dashboard**:
   - Open your browser to `http://localhost:5000`
   - The web interface provides real-time detection monitoring
   - Serial terminal for device output
   - Detection history and export capabilities

7. **Monitor device output** (optional):
   ```bash
   pio device monitor
   ```
