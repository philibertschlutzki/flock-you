# Contributing to Flock You

Vielen Dank für Ihr Interesse an der Mitarbeit am Flock You Projekt! 

## 🎯 Beiträge zur Blacklist (Known Device List)

### Blacklist-Konzept

Dieses Projekt verwendet eine **Blacklist-basierte Unknown-Detection**:
- **Known** (in Blacklist) → Gerät wird gefiltert (keine Ausgabe)
- **Unknown** (nicht in Blacklist) → Gerät wird als Detection gemeldet

Die Blacklist wird aus CSV-Dateien im `datasets/` Ordner generiert und enthält:
- WiFi SSIDs (exakte Strings)
- WiFi MAC-Adressen (6 Bytes)
- BLE MAC-Adressen (6 Bytes)
- OUIs (3-Byte Herstellerpräfixe)

### Blacklist-Dateien bearbeiten

Die Blacklist wird generiert aus CSV-Dateien in `datasets/`:

1. **CSV-Daten hinzufügen/bearbeiten:**
   - Platzieren Sie CSV-Dateien mit WiFi/BLE Gerätedaten in `datasets/`
   - Format: Spalten `type`, `ssid`, `netid` (MAC-Adresse)
   - WiFi-Einträge: `type` = leer oder `wifi`, mit `ssid` und `netid` (MAC)
   - BLE-Einträge: `type` = `BLE`, mit `netid` (MAC)

2. **Blacklist generieren:**
   ```bash
   python3 tools/generate_blacklist_from_csv.py
   ```
   
   Dies erzeugt:
   - `include/known_blacklist_generated.h`
   - `src/known_blacklist_generated.cpp`

3. **Kompilieren und flashen:**
   ```bash
   pio run --target upload
   ```

### Matching-Regeln

Die Blacklist verwendet **exakte Vergleiche ohne Wildcards oder Patterns**:

- **SSID:** Case-sensitive exakte String-Vergleiche
- **MAC:** 6-Byte binärer Vergleich (`memcmp`)
- **OUI:** 3-Byte binärer Vergleich der ersten Bytes
- **Keine Wildcards, keine Patterns, keine Substring-Matches**

### Pull Request Guidelines

1. **Beschreibung:** Erklären Sie, welche bekannten Geräte/Netzwerke zur Blacklist hinzugefügt werden
2. **Quellen:** Geben Sie Quellen an (z.B. eigene Scans, öffentliche Datenbanken)
3. **Datenschutz:** Stellen Sie sicher, dass keine privaten/sensiblen Daten in den CSVs enthalten sind
4. **Testing:** Testen Sie lokal, dass die Generierung funktioniert und die Geräte korrekt gefiltert werden

### Beispiel Pull Request

**Titel:** `Add public WiFi hotspots to blacklist`

**Beschreibung:**
```
Fügt bekannte öffentliche WiFi-Hotspots zur Blacklist hinzu.

- Quelle: Öffentliche SSID-Liste von Cafés/Flughäfen
- Anzahl: 50 SSIDs, 30 MAC-Adressen
- Tests: Lokal generiert und getestet
- Verifiziert: Bekannte Geräte werden korrekt gefiltert
```

## 🐛 Bug Reports

Bei Bug Reports bitte folgende Informationen angeben:
- Hardware: LILYGO T-Display-S3
- Firmware-Version
- Serial Log Output
- Schritte zur Reproduktion

## 💡 Feature Requests

Feature Requests sind willkommen! Bitte:
- Beschreiben Sie den Use Case
- Erklären Sie den erwarteten Nutzen
- Geben Sie mögliche Implementierungs-Ideen an

## 📝 Code Style

- Folgen Sie dem bestehenden Code-Stil
- Kommentare auf Deutsch oder Englisch
- Embedded-freundlich: kein `malloc()`, keine STL, stack-basiert
- Debug-Logs via `Serial.printf()` mit Prefix `[MODULE]`

## 📄 Lizenz

Durch Ihre Beiträge stimmen Sie zu, dass Ihre Arbeit unter der gleichen Lizenz wie das Projekt veröffentlicht wird.
