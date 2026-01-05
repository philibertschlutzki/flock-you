# Contributing to Flock You

Vielen Dank für Ihr Interesse an der Mitarbeit am Flock You Projekt! 

## 🎯 Beiträge zu Erkennungsmustern (Detection Patterns)

### Pattern-Anforderungen

Alle Erkennungsmuster müssen bestimmte Mindestanforderungen erfüllen, um zu verhindern, dass Patterns zu allgemein werden ("match-all Prevention").

#### WiFi SSID und BLE Device Name Patterns

**Anforderungen:**
- Mindestens **3 Literal-Zeichen** (ohne Wildcards)
- Keine reinen Wildcard-Patterns (`*`, `**`)

**Gültige Beispiele:**
```c
"RAVEN-*"        // ✅ 6 Literal-Zeichen + Wildcard
"*-Guest"        // ✅ 6 Literal-Zeichen + Wildcard
"Flock*Camera"   // ✅ 11 Literal-Zeichen + Wildcard
"MyNetwork"      // ✅ Exakt Match (10 Literal-Zeichen)
```

**Ungültige Beispiele:**
```c
"*"              // ❌ Nur Wildcard (match-all)
"a*"             // ❌ Nur 1 Literal-Zeichen
"ab*"            // ❌ Nur 2 Literal-Zeichen
```

#### MAC-Adress Patterns

**Anforderungen:**
- Mindestens **1 vollständiges Byte** (Format: `aa:`)
- Format: Hex-Bytes getrennt durch Doppelpunkte
- Wildcards erlaubt: `*` für beliebige Abschnitte

**Gültige Beispiele:**
```c
"aa:bb:cc:*"     // ✅ 3 vollständige Bytes + Wildcard
"58:8e:*"        // ✅ 2 vollständige Bytes + Wildcard
"aa:*"           // ✅ 1 vollständiges Byte + Wildcard
"*:dd:ee:ff"     // ✅ Wildcard + 3 vollständige Bytes
```

**Ungültige Beispiele:**
```c
"*"              // ❌ Kein vollständiges Byte
"a*"             // ❌ Kein vollständiges Byte
```

#### Raven Service UUID Patterns

**Anforderungen:**
- Mindestens **8 zusammenhängende Hex-Zeichen**
- Standard UUID-Format: `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`
- Wildcards erlaubt

**Gültige Beispiele:**
```c
"00003100-*"                                  // ✅ 8 Hex-Zeichen + Wildcard
"*-00805f9b34fb"                              // ✅ Wildcard + 12 Hex-Zeichen
"00003100-0000-1000-8000-00805f9b34fb"       // ✅ Vollständige UUID
```

**Ungültige Beispiele:**
```c
"*"              // ❌ Keine Hex-Zeichen
"0000*"          // ❌ Nur 4 Hex-Zeichen
```

### Wildcard-Syntax

- `*` = beliebige Zeichenfolge (inkl. leer)
- Alle Vergleiche sind **case-insensitiv**
- Patterns ohne `*` werden als exakte Matches interpretiert

### Pattern-Datei bearbeiten

Alle Patterns befinden sich in `include/detection_patterns.h`:

```c
// WiFi SSID Patterns
static const char* wifi_ssid_patterns[] = {
    "RAVEN-*",
    "Flock*",
    // Fügen Sie hier neue Patterns hinzu
};

// MAC-Adress Patterns
static const char* mac_prefixes[] = {
    "58:8e:81",
    "aa:bb:cc:*",
    // Fügen Sie hier neue Patterns hinzu
};

// BLE Device Name Patterns
static const char* device_name_patterns[] = {
    "Flock",
    "Penguin",
    // Fügen Sie hier neue Patterns hinzu
};

// Raven Service UUID Patterns
static const char* raven_service_uuids[] = {
    RAVEN_GPS_SERVICE,
    RAVEN_POWER_SERVICE,
    // Fügen Sie hier neue Patterns hinzu
};
```

### Pattern-Validierung testen

Nach dem Hinzufügen neuer Patterns:

1. **Kompilieren:**
   ```bash
   pio run
   ```

2. **Startup-Logs prüfen:**
   ```bash
   pio device monitor
   ```

3. **Suchen nach Validierungs-Ausgaben:**
   ```
   [PATTERN_CHECK] Starting pattern validation...
   [PATTERN_CHECK] WiFi SSID: 6 valid, 0 invalid
   [PATTERN_CHECK] MAC patterns: 20 valid, 0 invalid
   ```

4. **Bei Fehlern:**
   ```
   [PATTERN_CHECK] ERROR: Invalid WiFi SSID pattern #5: 'a*'
   [PATTERN_CHECK] Reason: SSID/Name pattern must have at least 3 literal characters
   ```

### Pull Request Guidelines

1. **Beschreibung:** Erklären Sie, welche Geräte/Netzwerke mit dem neuen Pattern erkannt werden
2. **Quellen:** Geben Sie Quellen an (z.B. beobachtete Deployments, Hersteller-Datenblätter)
3. **Spezifität:** Vermeiden Sie zu allgemeine Patterns (z.B. `"*"` oder `"test*"`)
4. **Testing:** Testen Sie Ihre Patterns lokal vor dem Pull Request

### Beispiel Pull Request

**Titel:** `Add RAVEN-R2 device pattern`

**Beschreibung:**
```
Fügt Pattern für neue Raven R2 Geräte hinzu, die mit "RAVEN-R2-" Präfix beworben werden.

- Pattern: "RAVEN-R2-*"
- Quelle: Beobachtungen in [Stadt/Gebiet]
- Validierung: ✅ 9 Literal-Zeichen
- Tests: Lokal getestet, matched "RAVEN-R2-123", "RAVEN-R2-XYZ"
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
