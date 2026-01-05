# Hardware Diagnostics Guide

**Bedienungsanleitung für den Hardware-Selbsttest**

Der Diagnostics-Modus testet alle Hardware-Komponenten des LILYGO T-Display-S3 Boards systematisch und zeigt die Ergebnisse sowohl auf dem Display als auch über die serielle Schnittstelle an.

---

## 🎯 Zweck

Der Hardware-Selbsttest dient dazu:
- **Neue Boards zu verifizieren**: Funktionieren alle Komponenten?
- **Fehlersuche**: Welches Bauteil verursacht Probleme?
- **Entwicklung**: Hardware-Status schnell überprüfen
- **Qualitätssicherung**: Konsistenter Test für alle Boards

---

## 🚀 Diagnostics-Modus starten

### Methode 1: Button beim Boot

1. **Board ausschalten** (USB-Kabel trennen)
2. **Button A gedrückt halten** (GPIO 0)
3. **USB-Kabel anschließen** (während Button gedrückt)
4. **Button 3 Sekunden gedrückt halten**
5. Display zeigt "DIAGNOSTICS" → Button loslassen

### Methode 2: Nach Reset

1. **Reset-Button drücken** am Board
2. **Sofort Button A gedrückt halten**
3. **3 Sekunden halten**
4. Diagnostics-Modus startet

### Erkennung

**Serial Monitor zeigt:**
```
[DIAGNOSTICS] Button A detected - checking hold time...
[DIAGNOSTICS] Button A held long enough - entering diagnostics mode

========================================
FLOCK YOU HARDWARE DIAGNOSTICS
LILYGO T-Display-S3
========================================
```

**Display zeigt:**
```
DIAGNOSTICS
Starting tests...
```

---

## 🧪 Test-Ablauf

Der Diagnostics-Modus führt **6 Tests** nacheinander durch:

### 1️⃣ Display Test (RGB + Text)

**Was wird getestet:**
- Display-Initialisierung
- Farbdarstellung (Schwarz, Rot, Grün, Blau, Weiß)
- Text-Rendering verschiedener Schriftgrößen
- TFT-Controller (ST7789) Kommunikation

**Dauer:** ~5 Sekunden

**Erwartetes Ergebnis:**
- Display zeigt nacheinander alle Farben
- Text ist lesbar und korrekt positioniert
- **PASS**: Alle Farben sichtbar, Text lesbar
- **FAIL**: Display bleibt schwarz oder Farben falsch

**Display-Ausgabe:**
```
DIAGNOSTICS
Test: display
[PASS]  ✓
RGB + Text
```

**Serial JSON:**
```json
{
  "type": "diagnostics_result",
  "test_name": "display",
  "status": "PASS",
  "timestamp_ms": 5234,
  "uptime_sec": 5.234,
  "details": "RGB + Text OK"
}
```

**Mögliche Fehler:**
- Display bleibt schwarz → Backlight-Pin falsch, Pins vertauscht
- Falsche Farben → RGB/BGR Order falsch, SPI-Problem
- Kein Signal → SPI-Pins (MOSI, SCLK, CS, DC, RST) prüfen

---

### 2️⃣ Backlight Test (PWM)

**Was wird getestet:**
- Backlight On/Off Schaltung
- PWM-Helligkeitssteuerung (0% → 100% → 0%)
- GPIO 38 Funktion

**Dauer:** ~5 Sekunden

**Erwartetes Ergebnis:**
- Backlight schaltet sichtbar aus und an
- Dimming-Rampe ist sichtbar (heller → dunkler)
- **PASS**: Backlight steuerbar
- **FAIL**: Backlight reagiert nicht

**Display-Ausgabe:**
```
DIAGNOSTICS
Test: backlight
[PASS]  ✓
PWM Control
```

**Mögliche Fehler:**
- Backlight immer an → GPIO 38 nicht verbunden oder falsch
- Kein PWM → Pin nicht PWM-fähig (sollte nicht passieren)

---

### 3️⃣ Button Test (A + B)

**Was wird getestet:**
- Button A (GPIO 0) Eingabe
- Button B (GPIO 14) Eingabe
- Debouncing-Logik
- Pull-Up Widerstände

**Dauer:** 10 Sekunden (Timeout)

**Interaktion erforderlich:**
- **Drücken Sie Button A** während des Tests
- **Drücken Sie Button B** während des Tests
- Live-Feedback auf dem Display

**Erwartetes Ergebnis:**
- Display zeigt "PRESSED" wenn Button gedrückt
- Display zeigt "Released" wenn Button losgelassen
- **PASS**: Beide Buttons wurden mindestens einmal erkannt
- **FAIL**: Mindestens ein Button reagiert nicht

**Display-Ausgabe während Test:**
```
Button Test
Press buttons A & B
(10 sec timeout)

Button A: PRESSED  ✓
Button B: Released
```

**Nach Test:**
```
DIAGNOSTICS
Test: buttons
[PASS]  ✓
A:OK B:OK
```

**Mögliche Fehler:**
- Button nicht erkannt → Pin nicht verbunden, falsche GPIO
- Immer "PRESSED" → Pull-Up fehlt, Pin kurzgeschlossen

---

### 4️⃣ WiFi Test (Promiscuous Mode)

**Was wird getestet:**
- WiFi-Modul Initialisierung
- Promiscuous Mode Aktivierung
- Channel Hopping (Kanal 1-13)
- Frame-Empfang (Beacons, Probe Requests)

**Dauer:** ~3 Sekunden

**Erwartetes Ergebnis:**
- WiFi empfängt Frames von Umgebung
- Channel Hopping funktioniert
- **PASS**: Mindestens 1 Frame empfangen
- **FAIL**: Keine Frames empfangen

**Display-Ausgabe:**
```
WiFi Test

Channel: 13
Frames: 1247
Last RSSI: -67 dBm
```

**Nach Test:**
```
DIAGNOSTICS
Test: wifi
[PASS]  ✓
Promiscuous Mode
```

**Serial JSON:**
```json
{
  "type": "diagnostics_result",
  "test_name": "wifi",
  "status": "PASS",
  "timestamp_ms": 18456,
  "uptime_sec": 18.456,
  "details": "Frames: 1247, RSSI: -67 dBm"
}
```

**Mögliche Fehler:**
- Keine Frames → WiFi-Modul defekt (selten)
- Sehr wenige Frames → Umgebung hat wenig WiFi-Aktivität (normal)

---

### 5️⃣ BLE Test (Device Scan)

**Was wird getestet:**
- BLE-Modul (NimBLE) Initialisierung
- Active Scan für 5 Sekunden
- Geräte-Erkennung
- Advertisement-Parsing

**Dauer:** ~5 Sekunden

**Erwartetes Ergebnis:**
- BLE findet Geräte in Umgebung (Smartphones, Smartwatches, etc.)
- Geräte-Informationen (MAC, Name, RSSI) werden angezeigt
- **PASS**: Mindestens 1 BLE-Gerät gefunden
- **FAIL**: Keine BLE-Geräte gefunden

**Display-Ausgabe während Scan:**
```
BLE Test
Scanning...

Devices: 12

Last device:
a4:c1:38:5f:2a:b1
iPhone von Max
RSSI: -54 dBm
```

**Nach Test:**
```
DIAGNOSTICS
Test: ble
[PASS]  ✓
Device Scan
```

**Serial JSON:**
```json
{
  "type": "diagnostics_result",
  "test_name": "ble",
  "status": "PASS",
  "timestamp_ms": 23892,
  "uptime_sec": 23.892,
  "details": "Found 12 devices, Last: a4:c1:38:5f:2a:b1"
}
```

**Mögliche Fehler:**
- Keine Geräte gefunden → Umgebung hat keine BLE-Geräte (selten)
- BLE Init fehlgeschlagen → BLE-Modul defekt (sehr selten)

---

### 6️⃣ Memory Test (PSRAM + Heap)

**Was wird getestet:**
- PSRAM Verfügbarkeit
- PSRAM Allocation Test (100 KB)
- Heap-Größe und freier Speicher
- Minimaler freier Heap

**Dauer:** ~2 Sekunden

**Erwartetes Ergebnis:**
- PSRAM ist verfügbar (ESP32-S3 Standard)
- PSRAM Allocation erfolgreich
- **PASS**: PSRAM funktioniert
- **FAIL**: PSRAM nicht verfügbar oder Allocation fehlgeschlagen

**Display-Ausgabe:**
```
Memory Test

Heap: 256 / 384 KB
Min Free: 248 KB

PSRAM: Available ✓
PSRAM: 7892 / 8192 KB
```

**Nach Test:**
```
DIAGNOSTICS
Test: memory
[PASS]  ✓
PSRAM + Heap
```

**Serial JSON:**
```json
{
  "type": "diagnostics_result",
  "test_name": "memory",
  "status": "PASS",
  "timestamp_ms": 26123,
  "uptime_sec": 26.123,
  "details": "Heap:256KB PSRAM:OK(7892KB)"
}
```

**Mögliche Fehler:**
- PSRAM nicht verfügbar → Board-Variante ohne PSRAM (ungewöhnlich)
- Allocation fehlgeschlagen → PSRAM defekt

---

## 📊 Test-Zusammenfassung

Nach allen Tests zeigt das Display:

```
SUMMARY

5 / 6 PASSED

Press RESET to reboot
```

**Farben:**
- **Grün**: Alle Tests bestanden (6/6)
- **Gelb**: Einige Tests bestanden (>50%)
- **Rot**: Viele Tests fehlgeschlagen (<50%)

**Serial JSON Summary:**
```
========================================
DIAGNOSTICS COMPLETE: 6/6 PASSED
========================================
```

---

## 🔍 Interpretation der Ergebnisse

### ✅ Alle Tests PASS (6/6)

**Bedeutung:** Hardware ist voll funktionsfähig
- Board kann für normale Nutzung verwendet werden
- Keine Hardware-Probleme erkannt

### ⚠️ Einzelner Test FAIL

**Display FAIL:**
- → Pin-Konfiguration prüfen (`tdisplay_s3_tft_espi_setup.h`)
- → Backlight-Pin verifizieren
- → Board-Revision vergleichen

**Backlight FAIL:**
- → GPIO 38 Verbindung prüfen
- → Hardware-Revision (andere Backlight-Pin?)

**Buttons FAIL:**
- → Weniger kritisch, Board funktioniert trotzdem
- → Buttons optional für Normalbetrieb

**WiFi FAIL:**
- → Sehr ungewöhnlich, WiFi-Modul defekt
- → Antenne prüfen (falls extern)

**BLE FAIL:**
- → Umgebung prüfen (andere BLE-Geräte in der Nähe?)
- → Sehr ungewöhnlich, BLE-Modul defekt

**Memory FAIL:**
- → Board-Variante ohne PSRAM?
- → Software funktioniert trotzdem (reduzierter Speicher)

### ❌ Mehrere Tests FAIL

**Bedeutung:** Schwerwiegendes Hardware-Problem
- → Board-Verbindungen prüfen
- → Falsches Board-Profil in `platformio.ini`?
- → Hardware defekt → Austausch erwägen

---

## 🛠️ Troubleshooting

### Display zeigt nichts während Diagnostics

**Checkliste:**
1. USB-Kabel verbunden? (Stromversorgung)
2. Backlight-Pin korrekt? (GPIO 38)
3. Serial Monitor zeigt Diagnostics-Meldungen?
4. Display-Pins in `tdisplay_s3_tft_espi_setup.h` korrekt?

### Serial Monitor zeigt keine Ausgabe

**Lösung:**
- Baudrate auf **115200** einstellen
- Richtigen COM-Port auswählen
- USB-Kabel mit Daten-Support verwenden

### Button A reagiert nicht

**Workaround:**
- Diagnostics manuell im Code aktivieren:
  ```cpp
  void setup() {
      // Immer Diagnostics starten (temporär)
      diagnostics_run_all_tests();
  }
  ```

### Test hängt / freezt

**Lösung:**
- Reset-Button drücken
- Neuer Upload versuchen
- Seriellen Monitor während Upload schließen

---

## 📝 JSON Log-Format

Alle Test-Ergebnisse werden als JSON über Serial ausgegeben:

```json
{
  "type": "diagnostics_result",
  "test_name": "display | backlight | buttons | wifi | ble | memory",
  "status": "PASS | FAIL",
  "timestamp_ms": 12345,
  "uptime_sec": 12.345,
  "details": "Optional: zusätzliche Informationen"
}
```

**Nutzung:**
- Logging in Datei: `pio device monitor > diagnostics.log`
- Parsing mit Tools: `jq`, Python, etc.
- Automatisierte Tests möglich

---

## 🔄 Aus Diagnostics-Modus beenden

**Methode 1: Reset-Button**
- Reset-Button am Board drücken
- Board startet normal (ohne Button A gedrückt)

**Methode 2: USB neu verbinden**
- USB-Kabel trennen
- USB-Kabel wieder verbinden
- Normaler Boot (ohne Button A)

**Methode 3: Erneut flashen**
```bash
pio run --target upload
```

---

## 📚 Weiterführende Informationen

**Code-Dateien:**
- `include/diagnostics.h` – Header mit Funktionsdeklarationen
- `src/diagnostics.cpp` – Implementierung aller Tests
- `src/main.cpp` – Integration (Boot-Check)

**Verwandte Dokumentation:**
- [Flash-Anleitung](FLASHING_LILYGO_TDISPLAY_S3_DE.md)
- [Projekt README](../README.md)
- [Roadmap](ROADMAP.md)

---

**Bei Fragen oder Problemen:**
- GitHub Issues: [github.com/philibertschlutzki/flock-you/issues](https://github.com/philibertschlutzki/flock-you/issues)

---

**Viel Erfolg beim Testen! 🧪**
