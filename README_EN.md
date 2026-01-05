# Flock You: Flock Safety Detection System

<img src="flock.png" alt="Flock You" width="300px">

**Professional surveillance camera detection for the LILYGO T-Display-S3 ESP32-S3 board**

## Overview

Flock You is an advanced detection system designed to identify Flock Safety surveillance cameras, Raven gunshot detectors, and similar surveillance devices using multiple detection methodologies.

The firmware target is the **LILYGO T-Display-S3 (ESP32-S3 + 1.9\" ST7789 LCD)**.

## Features

### Multi-Method Detection
- **WiFi Promiscuous Mode**: Captures probe requests and beacon frames
- **Bluetooth Low Energy (BLE) Scanning**: Monitors BLE advertisements
- **MAC Address Filtering**: Detects devices by known MAC prefixes
- **SSID Pattern Matching**: Identifies networks by specific names
- **Device Name Pattern Matching**: Detects BLE devices by advertised names
- **BLE Service UUID Detection**: Identifies Raven gunshot detectors by service UUIDs (NEW)

### Display Alert System (LILYGO T-Display-S3)
- **Status screen**: Current WiFi channel, BLE scan state, uptime, and device-in-range indicator
- **Alert screen**: Shows last detection (WiFi/BLE/Raven) with RSSI and a short label
- **Backlight control**: On/Off and PWM brightness control
- **Hardware diagnostics**: Full self-test of all components

### Comprehensive Output
- **JSON Detection Data**: Structured output with timestamps, RSSI, MAC addresses
- **Real-time Web Dashboard**: Live monitoring at `http://localhost:5000`
- **Serial Terminal**: Real-time device output in the web interface
- **Detection History**: Persistent storage and export capabilities (CSV, KML)
- **Device Information**: Full device details including signal strength and threat assessment
- **Detection Method Tracking**: Identifies which detection method triggered the alert

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
- Python 3.8+ (for web interface)
- USB-C cable for programming
- LILYGO T-Display-S3 board

### Setup Instructions
1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd flock-you
   ```

2. **Flash the firmware**:
   ```bash
   pio run --target upload
   ```

3. **Set up the web interface**:
   ```bash
   cd api
   python3 -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   pip install -r requirements.txt
   ```

4. **Start the web server**:
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
