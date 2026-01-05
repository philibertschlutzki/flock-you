# Flock You: Flock Safety Erkennungssystem

**Professionelle Überwachungskamera-Erkennung für das LILYGO T-Display-S3 Board**

---

## 📖 Über dieses Projekt

Flock You ist ein fortschrittliches Erkennungssystem zur Identifizierung von Flock Safety Überwachungskameras, Raven Schusserkennungsgeräten und ähnlichen Überwachungsgeräten mittels mehrerer Erkennungsmethoden.

Die **Zielhardware** ist das **LILYGO T-Display-S3** (ESP32-S3 mit 1.9" ST7789 Display).

---

## ✨ Features

### Multi-Methoden-Erkennung

- **WiFi Promiscuous Mode**: Erfasst Probe Requests und Beacon Frames
- **Bluetooth Low Energy (BLE) Scanning**: Überwacht BLE-Advertisements
- **MAC-Adress-Filterung**: Erkennt Geräte anhand bekannter MAC-Präfixe
- **SSID-Muster-Abgleich**: Identifiziert Netzwerke anhand spezifischer Namen
- **Gerätenamen-Erkennung**: Erkennt BLE-Geräte anhand beworbener Namen
- **BLE Service UUID Erkennung**: Identifiziert Raven Schusserkennungsgeräte anhand Service-UUIDs
- **Wildcard-Unterstützung (`*`)**: Flexible Pattern-Matching für alle Filter (SSID, MAC, UUID)

### Display-basiertes Alertsystem (LILYGO T-Display-S3)

- **Status-Screen**: Aktueller WiFi-Kanal, BLE-Scan-Status, Uptime, Gerät-in-Reichweite-Anzeige
- **Alert-Screen**: Zeigt letzte Erkennung (WiFi/BLE/Raven) mit RSSI und Kurztext
- **Backlight-Steuerung**: Ein/Aus und PWM-Helligkeitssteuerung
- **Hardware-Diagnostics**: Vollständiger Selbsttest aller Komponenten

### Umfassende Datenausgabe

- **JSON-Erkennungsdaten**: Strukturierte Ausgabe mit Timestamps, RSSI, MAC-Adressen
- **Echtzeit Web-Dashboard**: Live-Monitoring unter `http://localhost:5000`
- **Serial Terminal**: Echtzeit-Geräteausgabe im Web-Interface
- **Erkennungs-Historie**: Persistente Speicherung und Export-Möglichkeiten (CSV, KML)
- **Geräte-Informationen**: Vollständige Gerätedetails inkl. Signalstärke und Bedrohungsbewertung
- **Erkennungs-Methoden-Tracking**: Zeigt welche Methode den Alert ausgelöst hat

### Sicherheit & Validierung

- **Pattern-Validierung**: Automatische Prüfung aller Erkennungsmuster beim Boot
- **Mindest-Spezifität**: Verhindert zu allgemeine Patterns (match-all Prevention)
- **Graceful Degradation**: Ungültige Patterns werden übersprungen mit Warnung

---

## 🔧 Hardware-Anforderungen

### LILYGO T-Display-S3

- **Mikrocontroller**: ESP32-S3 mit PSRAM
- **Display**: 1.9" ST7789 TFT LCD (170x320 Pixel)
- **Wireless**: Dual WiFi/BLE Scanning
- **Buttons**: 2x Buttons (A und B) für Interaktion
- **Backlight**: PWM-steuerbare Hintergrundbeleuchtung
- **Konnektivität**: USB-C für Programmierung und Stromversorgung
- **Kein Audio**: Buzzer nicht vorhanden (Display-basierte Alerts)

**Bezugsquellen:**
- AliExpress, Amazon, Tindie
- Preis: ca. 15-25 EUR

---

## 📦 Installation

### Schnellstart für LILYGO T-Display-S3

```bash
# 1. Repository klonen
git clone https://github.com/philibertschlutzki/flock-you.git
cd flock-you

# 2. PlatformIO installieren (falls nicht vorhanden)
pip install -U platformio

# 3. Firmware kompilieren und flashen
pio run --target upload

# 4. Serial Monitor öffnen (optional)
pio device monitor
```

**Fertig!** Das Board startet und zeigt die Flock You UI auf dem Display.

### Ausführliche Anleitung

Für eine **detaillierte Schritt-für-Schritt-Anleitung** siehe:
📘 **[Flash-Anleitung (Deutsch)](docs/FLASHING_LILYGO_TDISPLAY_S3_DE.md)**

Die Anleitung erklärt:
- Installation aller Voraussetzungen (Git, Python, PlatformIO, VSCode)
- Repository klonen und Projekt öffnen
- Board anschließen und Port finden
- Firmware kompilieren und flashen
- Häufige Fehler und deren Lösungen
- Alternative mit Arduino IDE (nicht empfohlen)

---

## 🧪 Hardware-Diagnostics

Das Board verfügt über einen **integrierten Hardware-Selbsttest** der alle Komponenten überprüft.

### Diagnostics-Modus starten

1. **Board ausschalten** (USB-Kabel trennen)
2. **Button A gedrückt halten** (GPIO 0)
3. **USB-Kabel anschließen** (während Button gedrückt)
4. **3 Sekunden halten**
5. Display zeigt "DIAGNOSTICS"

### Getestete Komponenten

- ✅ **Display**: Farbflächen (RGB), Text-Rendering
- ✅ **Backlight**: On/Off, PWM Dimming (0%-100%-0%)
- ✅ **Buttons**: Button A + B Live-Test
- ✅ **WiFi**: Promiscuous Mode, Channel Hopping, Frame Counter
- ✅ **BLE**: Device Scan, Geräte-Zählung, letztes Gerät
- ✅ **Memory**: PSRAM Check, Heap Status

**Detaillierte Anleitung:** 📘 **[Hardware Diagnostics Guide (Deutsch)](docs/HARDWARE_DIAGNOSTICS_DE.md)**

---

## 🚀 Verwendung

### Nach dem Flash

1. **Display zeigt Status-Screen:**
   - WiFi Kanal
   - BLE Scan-Status
   - Uptime
   - "In range: NO/YES"

2. **Bei Erkennung:**
   - Display wechselt zu Alert-Screen (8 Sekunden)
   - Zeigt Erkennungstyp (WIFI/BLE/RAVEN)
   - Zeigt Details und RSSI

3. **Serial Monitor:**
   ```json
   {
     "timestamp": 12345,
     "protocol": "wifi",
     "detection_method": "probe_request",
     "ssid": "Flock-Camera-A4B2",
     "rssi": -67,
     "mac_address": "58:8e:81:a4:b2:c1",
     "alert_level": "HIGH",
     "device_category": "FLOCK_SAFETY"
   }
   ```

### Web-Dashboard (Optional)

```bash
# In api/ Verzeichnis wechseln
cd api

# Virtual Environment erstellen
python3 -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate

# Dependencies installieren
pip install -r requirements.txt

# Server starten
python flockyou.py

# Browser öffnen
# http://localhost:5000
```

**Dashboard-Features:**
- Live-Karte mit Erkennungen
- Echtzeit Serial Terminal
- Erkennungs-Historie
- CSV/KML Export

---

## 🎯 Erkennungs-Prinzip

### WiFi-Erkennung

Das System nutzt **WiFi Promiscuous Mode** um alle WiFi-Frames zu erfassen:

1. **Channel Hopping**: Wechselt zwischen Kanälen 1-13
2. **Frame-Typen**: Probe Requests (0x20) und Beacons (0x80)
3. **SSID-Matching**: Vergleicht SSID mit bekannten Mustern (Patterns)
4. **MAC-Präfix-Matching**: Vergleicht MAC-Adresse mit Hersteller-/Pattern-Liste

**Beispiel-Patterns (Wildcard `*`):**
```cpp
// SSID Patterns (Substring-Suche)
"*flock*", "*Flock*", "*FLOCK*", "*FS Ext Battery*", "*Penguin*", "*Pigvision*"

// MAC Patterns (Auszug)
"58:8e:81:*", "cc:cc:cc:*", "ec:1b:bd:*", "90:35:ea:*", "04:0d:84:*", ...
```

### BLE-Erkennung

Das System nutzt **NimBLE Active Scanning**:

1. **Advertisement Scan**: Empfängt BLE-Werbepakete
2. **Name-Matching**: Vergleicht Gerätenamen mit Patterns (Wildcard `*`)
3. **MAC-Präfix-Matching**: Identifiziert Hersteller
4. **Service UUID Detection**: Erkennt Raven-spezifische UUIDs

**Raven Service UUIDs:**
```cpp
"00003100-0000-1000-8000-00805f9b34fb"  // GPS Service
"00003200-0000-1000-8000-00805f9b34fb"  // Power Service
"00003300-0000-1000-8000-00805f9b34fb"  // Network Service
// ... und weitere
```

### False Positives vermeiden

⚠️ **Wichtig**: Das System basiert auf Heuristiken. Es kann nicht garantieren, dass ein erkanntes Gerät tatsächlich eine Flock Safety Kamera ist. Nutzen Sie das System verantwortungsvoll.

**Minimierung von False Positives:**
- Mehrfach-Kriterien: SSID + MAC kombiniert
- RSSI-Filter: Nur Geräte in Reichweite
- Confidence Scoring: 70-100 je nach Kriterien

---

## 📁 Projekt-Struktur

```
flock-you/
├── platformio.ini              # PlatformIO Konfiguration
├── README.md                   # Diese Datei (Deutsch)
├── include/
│   ├── config.h               # Zentrale Konfiguration & Pin-Mapping
│   ├── tdisplay_s3_tft_espi_setup.h  # TFT_eSPI Display-Config
│   ├── diagnostics.h          # Hardware-Diagnostics Header
│   ├── ui_display.h           # Display-UI Header
│   ├── wifi_sniffer.h         # WiFi Sniffer Header
│   ├── ble_scanner.h          # BLE Scanner Header
│   ├── detection_patterns.h   # Detection Patterns Header
│   └── output.h               # Output/Logging Header
├── src/
│   ├── main.cpp               # Haupt-Firmware
│   ├── diagnostics.cpp        # Hardware-Diagnostics Implementierung
│   ├── ui_display.cpp         # Display-UI Implementierung
│   ├── wifi_sniffer.cpp       # WiFi Sniffer Implementierung
│   ├── ble_scanner.cpp        # BLE Scanner Implementierung
│   └── output.cpp             # Output/Logging Implementierung
├── api/
│   ├── flockyou.py            # Python Web-Server
│   ├── requirements.txt       # Python Dependencies
│   └── templates/             # Web-UI Templates
└── docs/
    ├── FLASHING_LILYGO_TDISPLAY_S3_DE.md   # Flash-Anleitung
    ├── HARDWARE_DIAGNOSTICS_DE.md          # Diagnostics-Guide
    └── ROADMAP.md                           # Feature-Roadmap
```

---

## ⚙️ Konfiguration

### Compile-Time-Flags

Definiert in `platformio.ini`:

```ini
-DFLOCKYOU_NO_BUZZER=1       # Kein Buzzer (T-Display-S3)
-DFLOCKYOU_HAS_DISPLAY=1     # Display verfügbar
```

### Filter & Wildcards (Patterns)

Die Erkennung basiert auf **Pattern-Listen** (Filter), die in `include/detection_patterns.h` definiert sind.
Dort kannst du festlegen, **welche WiFi-/BLE-Signale** als „Treffer“ gelten (SSID, BLE-Name, MAC, Raven-UUID).

**Detaillierte Referenz (empfohlen):** 📘 **[Filter & Matching (Deutsch)](docs/FILTERS_AND_MATCHING_DE.md)**

#### Welche Filter gibt es?

Die Firmware kann (je nach Erkennungsquelle) über diese Listen matchen:

- **WiFi SSID Patterns** (`wifi_ssid_patterns[]`): Matcht WiFi-Namen (SSID).
- **MAC Patterns / OUI** (`mac_prefixes[]`): Matcht Hersteller-/Geräte-Präfixe oder Suffixe der MAC-Adresse.
- **BLE Name Patterns** (`device_name_patterns[]`): Matcht beworbene BLE-Gerätenamen.
- **Raven Service UUIDs** (`raven_service_uuids[]`): Matcht BLE-Service-UUIDs (z.B. Raven).

Wenn irgendein Filter passt, löst das System eine Erkennung aus (WiFi/BLE/Raven).

#### Wildcard-Suche (`*`) verstehen

Alle Pattern-Listen unterstützen `*` als Wildcard:

- `*` = beliebige Zeichenfolge (auch leer)
- Matching ist **case-insensitiv** (Groß-/Kleinschreibung egal)

Beispiele:
- SSID: `RAVEN-*`, `*Guest`, `*flock*`
- MAC: `58:8e:*`, `aa:bb:cc:*`, `*:dd:ee:ff`
- UUID: `00003100-*`, `*-00805f9b34fb`

**Tipp für Anfänger:** Für „Substring-Suche“ einfach vorne und hinten `*` setzen, z.B. `*flock*`.

#### Filter-Modus wählen: LEGACY vs. ALLOWLIST

Es gibt zwei Betriebsmodi:

- **LEGACY**: Nutzt die „Standard-/Heuristik“-Patternlisten (für typische Erkennung).
- **ALLOWLIST (Test/Labor)**: Nutzt nur deine eigenen Allowlist-Einträge (ideal zum Debuggen und zum Reduzieren von False Positives).

Du stellst den Modus per Compile-Flag in `platformio.ini` um:

```ini
; LEGACY (Standard-Patterns)
build_flags =
  -DFLOCKYOU_FILTER_MODE=FLOCKYOU_FILTER_MODE_LEGACY
```

```ini
; ALLOWLIST (nur eigene Geräte/Patterns)
build_flags =
  -DFLOCKYOU_FILTER_MODE=FLOCKYOU_FILTER_MODE_ALLOWLIST
```

> Hinweis: Der Modus beeinflusst, **welche** Pattern-Arrays aktiv ausgewertet werden.

#### ALLOWLIST richtig benutzen (wichtig)

Im ALLOWLIST-Modus solltest du die Allowlist-Arrays so anpassen, dass **nur** deine Testgeräte matchen.

Beispiel: Nur ein bestimmtes WiFi matchen (SSID enthält „MyPhoneHotspot“):

```cpp
static const char* wifi_ssid_patterns[] = {
  "*MyPhoneHotspot*",
  ""  // leer = deaktiviert/ignoriert
};
```

Beispiel: Nur Geräte mit bestimmtem MAC-Präfix matchen:

```cpp
static const char* mac_prefixes[] = {
  "58:8e:*",
  ""
};
```

Beispiel: Nur ein BLE-Name matchen:

```cpp
static const char* device_name_patterns[] = {
  "*MySensor*",
  ""
};
```

#### Wildcard-Suche testweise „breit“ aktivieren

Wenn du prüfen willst, ob die Wildcard-Logik grundsätzlich funktioniert, setze testweise ein Pattern, das sicher vorkommt (z.B. dein Hotspot-Name oder ein typischer String wie `*Phone*`).
Danach wieder spezifischer werden, sonst bekommst du sehr viele Treffer.

Pragmatischer Testablauf:
1. Modus auf **ALLOWLIST** stellen.
2. In `include/detection_patterns.h` bei **SSID** ein Pattern setzen, das sicher vorkommt (z.B. dein Hotspot).
3. Flashen (`pio run --target upload`).
4. Prüfen, ob im Serial/Web-Dashboard Treffer erscheinen.

### Pin-Mapping (LILYGO T-Display-S3)

Definiert in `include/config.h`:

```cpp
#define TFT_BACKLIGHT_PIN 38  // Backlight PWM
#define BUTTON_A_PIN 0        // Button A (Diagnostics)
#define BUTTON_B_PIN 14       // Button B (zukünftig)
```

TFT SPI Pins in `include/tdisplay_s3_tft_espi_setup.h`:

```cpp
#define TFT_MOSI 17
#define TFT_SCLK 18
#define TFT_CS   6
#define TFT_DC   7
#define TFT_RST  5
#define TFT_BL   38
```

### WiFi & BLE Konfiguration

```cpp
#define MAX_CHANNEL 13                // WiFi Kanäle 1-13
#define CHANNEL_HOP_INTERVAL 500      // 500ms pro Kanal
#define BLE_SCAN_DURATION 1           // 1 Sekunde BLE Scan
#define BLE_SCAN_INTERVAL 5000        // 5 Sekunden Pause
```

---

## 🗺️ Roadmap

Siehe **[ROADMAP.md](docs/ROADMAP.md)** für detaillierte Feature-Planung.

**Kurzfristig (v0.2):**
- Persistent Logging (SD-Karte)
- CSV/KML Export
- Filter/Whitelist

**Mittelfristig (v0.3-v0.5):**
- Web-UI Optimierung
- Performance-Verbesserungen
- Pattern Database erweitern

**Langfristig (v1.0+):**
- OTA Updates
- Plugin-System
- GPS Integration

---

## 🤝 Beitragen

Wir freuen uns über Beiträge!

**Bereiche:**
- 🐛 Bug Reports: [GitHub Issues](https://github.com/philibertschlutzki/flock-you/issues)
- 💡 Feature Requests: [GitHub Discussions](https://github.com/philibertschlutzki/flock-you/discussions)
- 🔧 Pull Requests: Code, Dokumentation, Patterns
- 📚 Dokumentation: Übersetzungen, Tutorials

**Contribution Guidelines:**
- Kleine, reviewbare Commits
- Dokumentation auf Deutsch (Code-Kommentare)
- PlatformIO Build muss funktionieren (`pio run`)

---

## 📄 Lizenz

[Lizenz einfügen - z.B. MIT, GPL, etc.]

---

## ⚠️ Disclaimer

Dieses Projekt dient **ausschließlich zu Bildungs- und Forschungszwecken**. Die Nutzung von WiFi Promiscuous Mode und BLE Scanning unterliegt lokalen Gesetzen. Nutzer sind selbst verantwortlich für die Einhaltung geltender Gesetze.

Das System kann **keine 100%ige Genauigkeit garantieren**. Erkennungen basieren auf Heuristiken und können False Positives/Negatives produzieren.

---

## 🙏 Danksagungen

- **LILYGO** für das hervorragende T-Display-S3 Board
- **Bodmer** für die TFT_eSPI Library
- **h2zero** für NimBLE-Arduino
- **Community** für Pattern-Datenbank und Testing

---

## 📞 Kontakt & Support

- **GitHub**: [philibertschlutzki/flock-you](https://github.com/philibertschlutzki/flock-you)
- **Issues**: [Bug Reports](https://github.com/philibertschlutzki/flock-you/issues)
- **Discussions**: [Community Forum](https://github.com/philibertschlutzki/flock-you/discussions)

---

**Viel Erfolg mit Flock You! 🦜**
