# Flock You: WiFi/BLE Unknown-Device Detection

**Firmware für das LILYGO T-Display-S3 (ESP32-S3) zur Erkennung unbekannter WiFi- und BLE-Geräte über Promiscuous- und Active-Scanning.**

---

## 📖 Über dieses Projekt

Flock You überwacht die Umgebung mittels WiFi Promiscuous Mode und BLE Scanning und **meldet ausschließlich unbekannte Geräte** (d.h. Geräte, die nicht in einer Blacklist enthalten sind).

Die Zielhardware ist das **LILYGO T-Display-S3** (ESP32-S3 mit 1.9" ST7789 Display).

---

## ✨ Features

### Unknown-only Detection (Blacklist)

- **WiFi Promiscuous Mode**: Erfasst Probe Requests und Beacon Frames.
- **Bluetooth Low Energy (BLE) Active Scanning**: Überwacht BLE-Advertisements.
- **Blacklist-basierte Filterung**: Bekannte Geräte werden gefiltert; nur Unknowns werden gemeldet.
- **Matching ohne Wildcards/Patterns**:
  - SSID: exakter, **case-sensitiver** Match
  - MAC: 6-Byte Vergleich
  - OUI: 3-Byte Prefix (Herstellerpräfix)
- **Session Filter (RAM-only)**: Laufzeit-Blacklist-Erweiterung (z.B. für spätere WebGUI/API), Reset bei Reboot.

### Display UI (LILYGO T-Display-S3)

- Status-Screen (WiFi-Kanal, BLE-Status, Uptime)
- Alert-Screen bei Erkennung (WiFi/BLE, RSSI, Kurztext)
- Backlight-Steuerung (Ein/Aus, PWM)
- Hardware-Diagnostics (Selftest)

### Ausgabe

- JSON über Serial (für Logging/Weiterverarbeitung)
- Optional: Web-Dashboard im `api/` Ordner (konsumiert Serial-JSON)

---

## 🔧 Hardware-Anforderungen

### LILYGO T-Display-S3

- ESP32-S3 mit PSRAM
- 1.9" ST7789 TFT (170x320)
- WiFi + BLE
- 2 Buttons (A/B)

---

## 📦 Installation

### Schnellstart

```bash
# 1) Repository klonen
git clone https://github.com/philibertschlutzki/flock-you.git
cd flock-you

# 2) PlatformIO installieren (falls nicht vorhanden)
pip install -U platformio

# 3) Blacklist generieren (erforderlich für den Build)
python3 tools/generate_blacklist_from_csv.py

# 4) Firmware kompilieren und flashen
pio run --target upload

# 5) Serial Monitor öffnen (optional)
pio device monitor
```

Wenn du `datasets/` änderst, muss die Blacklist erneut generiert werden.

---

## 🎯 Erkennungs-Prinzip

### Grundlogik

- **Known** (in Blacklist/Session Filter) → wird verworfen/ignoriert.
- **Unknown** (nicht in Blacklist/Session Filter) → wird als Detection ausgegeben.

### Detection-Typen

- WiFi: z.B. `probe_request_unknown`, `beacon_unknown`
- BLE: `unknown`

Details (inkl. Matching-Regeln und Session Filter API) sind in der Filter-Dokumentation beschrieben:
📘 **[Blacklist, Filter & Matching (Deutsch)](docs/FILTERS_AND_MATCHING_DE.md)**

---

## 📁 Projekt-Struktur (Auszug)

```
flock-you/
├── platformio.ini
├── README.md
├── datasets/                       # Input für Blacklist-Generator (CSV)
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
│   ├── known_blacklist_generated.h  # generiert
│   └── output.h
├── src/
│   ├── main.cpp
│   ├── diagnostics.cpp
│   ├── ui_display.cpp
│   ├── wifi_sniffer.cpp
│   ├── ble_scanner.cpp
│   ├── blacklist.cpp
│   ├── session_filter.cpp
│   ├── known_blacklist_generated.cpp # generiert
│   └── output.cpp
└── docs/
    ├── FILTERS_AND_MATCHING_DE.md
    ├── FLASHING_LILYGO_TDISPLAY_S3_DE.md
    ├── HARDWARE_DIAGNOSTICS_DE.md
    └── ROADMAP.md
```

---

## 🤝 Beitragen

Wir freuen uns über Beiträge!

- 🐛 Bug Reports: https://github.com/philibertschlutzki/flock-you/issues
- 💡 Feature Requests: https://github.com/philibertschlutzki/flock-you/discussions
- 🔧 Pull Requests: Code, Dokumentation

---

## 📄 Lizenz

[Lizenz einfügen - z.B. MIT, GPL, etc.]

---

## ⚠️ Disclaimer

Dieses Projekt dient ausschließlich zu Bildungs- und Forschungszwecken. Die Nutzung von WiFi Promiscuous Mode und BLE Scanning unterliegt lokalen Gesetzen. Nutzer sind selbst verantwortlich für die Einhaltung geltender Gesetze.
