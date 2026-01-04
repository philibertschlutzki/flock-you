# Flash-Anleitung: LILYGO T-Display-S3

**Anfängerfreundliche Schritt-für-Schritt-Anleitung**

Diese Anleitung erklärt detailliert, wie Sie die Flock You Firmware auf Ihr **LILYGO T-Display-S3** Board flashen können. Sie benötigen keine Vorkenntnisse – wir führen Sie durch jeden Schritt.

---

## 📋 Voraussetzungen

### Hardware
- **LILYGO T-Display-S3** Board (ESP32-S3 mit 1.9" ST7789 Display)
- **USB-C Kabel** (Daten + Ladung, kein reines Ladekabel!)
- Computer (Windows, macOS oder Linux)

### Software
- **Git** für das Klonen des Repositories
- **Python 3.8+** (für PlatformIO)
- **PlatformIO** Build-System
- **Visual Studio Code** (empfohlen) oder PlatformIO Core (CLI)

---

## 🚀 Schritt 1: Software installieren

### 1.1 Git installieren

**Windows:**
- Download von [git-scm.com](https://git-scm.com/download/win)
- Installer ausführen, Standardeinstellungen verwenden
- Git Bash öffnen zum Testen: `git --version`

**macOS:**
```bash
# Mit Homebrew (empfohlen)
brew install git

# Oder Xcode Command Line Tools
xcode-select --install
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install git
```

### 1.2 Python installieren

**Windows:**
- Download von [python.org](https://www.python.org/downloads/)
- ⚠️ **WICHTIG**: "Add Python to PATH" ankreuzen!
- Installation durchführen
- Testen in CMD: `python --version`

**macOS:**
```bash
# Python 3 ist oft vorinstalliert, sonst:
brew install python3
```

**Linux:**
```bash
sudo apt install python3 python3-pip
```

### 1.3 Visual Studio Code + PlatformIO (EMPFOHLEN)

**Option A: VSCode mit PlatformIO Extension (Anfänger-freundlich)**

1. **Visual Studio Code** installieren: [code.visualstudio.com](https://code.visualstudio.com/)
2. VSCode öffnen
3. Extension-Symbol in der linken Leiste klicken (vier Quadrate)
4. Nach "PlatformIO IDE" suchen
5. "Install" klicken
6. Nach Installation VSCode neu starten

**Option B: PlatformIO Core (Kommandozeile, für Fortgeschrittene)**

```bash
# Mit pip installieren
pip install -U platformio

# Oder
python3 -m pip install -U platformio
```

---

## 📦 Schritt 2: Repository klonen

Öffnen Sie ein Terminal (Git Bash auf Windows, Terminal auf macOS/Linux):

```bash
# In Ihr gewünschtes Arbeitsverzeichnis wechseln
cd ~/Projekte  # oder z.B. C:\Users\IhrName\Projekte

# Repository klonen
git clone https://github.com/philibertschlutzki/flock-you.git
cd flock-you
```

---

## 🔌 Schritt 3: Board anschließen

1. **USB-C Kabel** an das LILYGO T-Display-S3 Board anschließen
2. Anderen USB-Stecker mit dem Computer verbinden
3. Board sollte kurz aufleuchten (Display oder LED)

### Port finden

**Windows:**
- Geräte-Manager öffnen (`Win + X` → Geräte-Manager)
- Unter "Anschlüsse (COM & LPT)" nachsehen
- Notieren Sie sich "COM3", "COM4", etc.
- Falls kein Port erscheint → Treiber installieren (siehe unten)

**macOS:**
```bash
ls /dev/cu.*
# Suchen Sie nach: /dev/cu.usbserial-XXXX oder /dev/cu.SLAB_USBtoUART
```

**Linux:**
```bash
ls /dev/ttyUSB* /dev/ttyACM*
# Normalerweise: /dev/ttyUSB0 oder /dev/ttyACM0

# Berechtigungen setzen (einmalig):
sudo usermod -a -G dialout $USER
# Danach abmelden und neu anmelden!
```

### Treiber installieren (falls nötig)

Falls Ihr Board nicht erkannt wird:

**Windows:**
- CP210x USB to UART Driver: [Silicon Labs](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- CH340 Driver: [WCH](http://www.wch.cn/downloads/CH341SER_EXE.html)

**macOS:**
- Meist automatisch erkannt
- Falls nicht: CP210x Driver von Silicon Labs

**Linux:**
- Treiber sind normalerweise im Kernel enthalten

---

## 🔨 Schritt 4: Firmware kompilieren und flashen

### Mit VSCode + PlatformIO (empfohlen)

1. **Projekt öffnen:**
   - VSCode starten
   - `File → Open Folder`
   - `flock-you` Ordner auswählen

2. **PlatformIO initialisieren:**
   - VSCode öffnet das Projekt
   - Warten bis PlatformIO das Projekt lädt (Status-Leiste unten)
   - Evtl. werden automatisch Abhängigkeiten heruntergeladen

3. **Bauen (Build):**
   - Unten in der Statusleiste: "✓" Symbol (Build) klicken
   - **ODER**: `Strg+Alt+B` / `Cmd+Alt+B`
   - **ODER**: Terminal → Run Task → PlatformIO: Build
   
   ⏱️ **Erste Kompilierung dauert 5-10 Minuten** (Downloads + Kompilierung)

4. **Flashen (Upload):**
   - Board per USB-C verbunden?
   - Unten in der Statusleiste: "→" Symbol (Upload) klicken
   - **ODER**: `Strg+Alt+U` / `Cmd+Alt+U`
   - **ODER**: Terminal → Run Task → PlatformIO: Upload

5. **Serial Monitor öffnen:**
   - Unten in der Statusleiste: "🔌" Symbol (Serial Monitor)
   - **ODER**: `Strg+Alt+S` / `Cmd+Alt+S`

### Mit Kommandozeile (PlatformIO Core)

```bash
# 1. In Projekt-Verzeichnis wechseln
cd flock-you

# 2. Build (Kompilieren)
pio run

# 3. Upload (Flashen)
pio run --target upload

# 4. Serial Monitor (Ausgabe ansehen)
pio device monitor
```

---

## ✅ Schritt 5: Erfolgskontrolle

### Was sollte passieren?

1. **Beim Flashen:**
   - Terminal zeigt Fortschritt
   - "Connecting..." erscheint
   - "Writing at 0x..." zeigt Upload-Fortschritt
   - "Hard resetting via RTS pin..." = Erfolg!

2. **Nach dem Flash:**
   - Display zeigt "Flock You"
   - Status-Informationen werden angezeigt
   - WiFi Channel, BLE Status, Uptime

3. **Serial Monitor zeigt:**
   ```
   Flock Squawk Enhanced Detection System...
   WiFi promiscuous mode enabled on channel 1
   BLE scanner initialized
   System ready - hunting for Flock Safety devices...
   ```

### Diagnostics-Modus testen

Beim Boot **Button A gedrückt halten** (3 Sekunden):
- Display zeigt "DIAGNOSTICS"
- Alle Hardware-Tests werden durchgeführt
- Ergebnisse: PASS/FAIL für jeden Test

---

## ❌ Häufige Fehler & Lösungen

### Problem: "Port busy" / "Port belegt"

**Lösung:**
- Schließen Sie alle Serial Monitor Programme
- Trennen und verbinden Sie das USB-Kabel neu
- Starten Sie VSCode/Terminal neu

### Problem: "Failed to connect" / "Verbindung fehlgeschlagen"

**Lösung:**
1. **Boot-Modus erzwingen:**
   - USB-Kabel trennen
   - Button "BOOT" (oft GPIO0) **gedrückt halten**
   - USB-Kabel anschließen (während Button gedrückt)
   - Button loslassen
   - Upload erneut versuchen

2. **USB-Kabel prüfen:**
   - Verwenden Sie ein **Daten-Kabel** (kein reines Ladekabel!)
   - Testen Sie ein anderes Kabel

3. **USB-Port wechseln:**
   - Versuchen Sie einen anderen USB-Port
   - Vermeiden Sie USB-Hubs, nutzen Sie direkte PC-Ports

### Problem: "Wrong COM port" / "Falscher Port"

**Lösung Windows:**
```bash
# Port manuell angeben
pio run --target upload --upload-port COM5
```

**Lösung macOS/Linux:**
```bash
# Port manuell angeben
pio run --target upload --upload-port /dev/ttyUSB0
```

### Problem: "No display output" / "Display bleibt schwarz"

**Mögliche Ursachen:**

1. **Backlight aus:**
   - Backlight-Pin eventuell falsch konfiguriert
   - Prüfen Sie `include/tdisplay_s3_tft_espi_setup.h`
   - GPIO 38 sollte `TFT_BL` sein

2. **Falsche Pin-Konfiguration:**
   - Es gibt verschiedene Board-Revisionen!
   - Vergleichen Sie Ihre Pins mit `tdisplay_s3_tft_espi_setup.h`

3. **Rotation falsch:**
   - In `src/main.cpp`: `tft.setRotation(1)` 
   - Versuchen Sie Werte 0, 1, 2, 3

**Test-Code für Display:**
```cpp
tft.init();
tft.setRotation(1);
tft.fillScreen(TFT_RED);    // Sollte rot sein
pinMode(38, OUTPUT);         // Backlight
digitalWrite(38, HIGH);      // Backlight an
```

### Problem: "Permission denied" (Linux)

**Lösung:**
```bash
# User zur dialout-Gruppe hinzufügen
sudo usermod -a -G dialout $USER

# Abmelden und neu anmelden (oder neu starten)
# ODER temporär:
sudo chmod 666 /dev/ttyUSB0
```

### Problem: Build-Fehler "Platform espressif32 not installed"

**Lösung:**
```bash
# PlatformIO Plattform manuell installieren
pio platform install espressif32
```

### Problem: "Out of memory" während Kompilierung

**Lösung:**
- Kompilierung benötigt viel RAM
- Schließen Sie andere Programme
- Verwenden Sie kleinere Partition: `board_build.partitions = default.csv`

---

## 🔄 Schritt 6: Firmware aktualisieren

Um eine neue Version zu flashen:

```bash
# Repository aktualisieren
cd flock-you
git pull

# Neu kompilieren und flashen
pio run --target upload
```

---

## 📱 Alternative: Arduino IDE (nicht empfohlen)

Falls Sie Arduino IDE bevorzugen (weniger getestet):

1. Arduino IDE installieren
2. ESP32 Board Support: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Board auswählen: "ESP32-S3 Dev Module"
4. Bibliotheken installieren:
   - NimBLE-Arduino
   - ArduinoJson
   - TFT_eSPI
5. TFT_eSPI User_Setup konfigurieren (komplex!)

⚠️ **Arduino IDE ist komplizierter** – wir empfehlen PlatformIO!

---

## 🆘 Weitere Hilfe

**Probleme beim Flash?**

1. Lesen Sie die Fehlermeldung genau
2. Suchen Sie in den "Häufigen Fehlern" oben
3. Prüfen Sie:
   - USB-Kabel (Daten-fähig?)
   - Treiber installiert?
   - Port korrekt?
   - Boot-Modus versucht?

**Immer noch Probleme?**
- GitHub Issues: [github.com/philibertschlutzki/flock-you/issues](https://github.com/philibertschlutzki/flock-you/issues)
- Beschreiben Sie detailliert:
  - Betriebssystem
  - Fehlermeldung (vollständig kopieren!)
  - Was Sie bereits versucht haben

---

## 📚 Weiterführende Dokumentation

- [Hardware Diagnostics Guide](HARDWARE_DIAGNOSTICS_DE.md) – Diagnose-Modus nutzen
- [ROADMAP](ROADMAP.md) – Geplante Features
- [README](../README.md) – Projekt-Übersicht

---

**Viel Erfolg beim Flashen! 🚀**
