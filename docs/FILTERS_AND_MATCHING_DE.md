# Filter & Matching (WiFi + BLE)

Diese Anleitung beschreibt, wie die Erkennungslogik für WiFi- und BLE-Events über Listen/Regeln angepasst werden kann.
Ziel: Regeln sind **datengetrieben** (Listen), und die Scan-Logik bleibt stabil.

> Stand im Code (main): Die Filter arbeiten derzeit überwiegend mit **Substring-Matching** (case-insensitiv) sowie MAC-Präfix-Vergleich auf den ersten 3 Bytes.

---

## 1) Schnellstart (Anfänger)

### Wo wird aktuell konfiguriert?
Die Filterlisten sind im Projekt bereits zentral in `include/detection_patterns.h` abgelegt:
- `wifi_ssid_patterns[]` (WiFi SSID-Substring)
- `mac_prefixes[]` (MAC/OUI-Präfixe im Format `aa:bb:cc`)
- `device_name_patterns[]` (BLE Name-Substring)
- `raven_service_uuids[]` + `RAVEN_*_SERVICE` (Raven Service UUIDs)

Für die meisten Anpassungen reicht es, **nur diese Arrays** zu erweitern.

### Änderungen in 3 Schritten
1. `include/detection_patterns.h` öffnen.
2. Passende Liste erweitern (siehe unten).
3. Build prüfen: `pio run`.

---

## 2) MAC-Adress-Filterung (OUI / Präfix)

### Was wird verglichen?
Im aktuellen Code wird aus der MAC-Adresse nur der Herstellerpräfix (OUI) gebildet: `aa:bb:cc` (erste 3 Bytes), und dann case-insensitiv gegen `mac_prefixes[]` verglichen.

> Hinweis: „OUI“ bezeichnet die ersten 24 Bit einer MAC-Adresse (3 Bytes). [web:14]

### Liste anpassen (Anfänger)
In `include/detection_patterns.h` unter `mac_prefixes[]` neue Einträge hinzufügen:
```c
static const char* mac_prefixes[] = {
    "58:8e:81",
    "e4:aa:ea",
    "aa:bb:cc" // neuer Präfix
};
```

### Häufige Fehler
- Falsches Format: Es muss exakt `aa:bb:cc` sein (8 Zeichen inkl. Doppelpunkte).
- Einträge ohne Komma.

---

## 3) SSID-Muster-Abgleich (WiFi)

### Aktueller Stand (Substring)
Im WiFi-Sniffer wird geprüft, ob die SSID **einen** der Einträge aus `wifi_ssid_patterns[]` enthält (case-insensitiv).
Beispiel: Ein Pattern `Flock` matcht auch `Flock-123`.

### Liste anpassen (Anfänger)
In `include/detection_patterns.h` unter `wifi_ssid_patterns[]` erweitern:
```c
static const char* wifi_ssid_patterns[] = {
    "flock",
    "Pigvision",
    "MyTestSSID" // neu
};
```

---

## 4) BLE Gerätenamen-Erkennung

### Aktueller Stand (Substring)
Im BLE-Scanner wird geprüft, ob der beworbene Gerätename **einen** der Einträge aus `device_name_patterns[]` enthält (case-insensitiv).

### Liste anpassen (Anfänger)
In `include/detection_patterns.h` unter `device_name_patterns[]` erweitern:
```c
static const char* device_name_patterns[] = {
    "Flock",
    "Penguin",
    "MyBLEDevice" // neu
};
```

---

## 5) BLE Service UUID Erkennung (Raven)

### Aktueller Stand (exakter UUID-String)
Raven-Geräte werden über Service-UUIDs erkannt:
- Der Scanner liest alle beworbenen Service UUIDs.
- Jede UUID wird zu einem String normalisiert.
- Dann wird **exakt** (case-insensitiv) gegen `raven_service_uuids[]` verglichen.

### Liste anpassen (Anfänger)
In `include/detection_patterns.h`:
1. Neue UUID als `#define` ergänzen (empfohlen, damit sie benannt ist).
2. Diese UUID in `raven_service_uuids[]` aufnehmen.

Beispiel:
```c
#define RAVEN_NEW_SERVICE "00003600-0000-1000-8000-00805f9b34fb"

static const char* raven_service_uuids[] = {
    RAVEN_DEVICE_INFO_SERVICE,
    RAVEN_NEW_SERVICE
};
```


---

## 6) Wildcard-Syntax (Neu implementiert)

Das System unterstützt jetzt **vollständige Wildcard-Patterns** für alle Erkennungsmuster (WiFi SSID, BLE Name, MAC-Adressen, Raven Service UUIDs).

### Was ist ein Wildcard?

- `*` = beliebige Zeichenfolge (inkl. leer)
- Alle Vergleiche sind **case-insensitiv**
- Patterns ohne `*` werden als exakte Matches interpretiert (Backward-Compatible)

### Wildcard-Beispiele für SSID und BLE-Namen

```c
static const char* wifi_ssid_patterns[] = {
    "RAVEN-*",        // Matcht: RAVEN-123, RAVEN-XYZ, RAVEN-TEST
    "*-Guest",        // Matcht: MyNetwork-Guest, Office-Guest
    "Flock*Camera",   // Matcht: FlockStreetCamera, Flock_Camera
    "MyNetwork"       // Matcht: nur exakt "MyNetwork" (kein Wildcard)
};
```

**Anwendungsfälle:**
- `"RAVEN-*"` - Alle Raven-Geräte mit beliebigem Suffix
- `"*Guest"` - Alle Gast-Netzwerke (endet mit "Guest")
- `"Test*Device"` - Testgeräte mit variablem Mittelteil
- `"*FLOCK*"` - Beliebiger Text mit "FLOCK" darin

### Wildcard-Beispiele für MAC-Adressen

```c
static const char* mac_prefixes[] = {
    "aa:bb:cc:*",     // Matcht: aa:bb:cc:dd:ee:ff
    "58:8e:*",        // Matcht: alle 58:8e:xx:xx:xx:xx (Hersteller-OUI)
    "*:dd:ee:ff",     // Matcht: alle MACs mit Suffix dd:ee:ff
    "aa:*:ff"         // Matcht: aa:bb:ff, aa:cc:dd:ee:ff
};
```

**Hinweis:** MAC-Patterns müssen mindestens 1 vollständiges Byte enthalten (z.B. `"aa:"`, `"bb:cc:*"`).

### Wildcard-Beispiele für Service UUIDs (Raven)

```c
static const char* raven_service_uuids[] = {
    "00003100-*",                                    // Matcht: alle GPS Service Variants
    "*-00805f9b34fb",                                // Matcht: alle mit diesem Suffix
    "00003100-*-00805f9b34fb",                       // Matcht: GPS mit spezifischem Suffix
    "00003100-0000-1000-8000-00805f9b34fb"          // Exakt Match (kein Wildcard)
};
```

**Hinweis:** UUID-Patterns müssen mindestens 8 zusammenhängende Hex-Zeichen enthalten.

### Mindest-Spezifität (Sicherheits-Constraints)

Um zu verhindern, dass Patterns zu allgemein werden ("match-all"), gibt es folgende Regeln:

1. **SSID/BLE Name:** Mindestens **3 Literal-Zeichen**
   - ✅ Gültig: `"RAVEN-*"`, `"abc*"`, `"*xyz"`
   - ❌ Ungültig: `"*"`, `"a*"`, `"ab*"`

2. **MAC-Adresse:** Mindestens **1 vollständiges Byte** (`aa:`)
   - ✅ Gültig: `"aa:*"`, `"bb:cc:*"`, `"*:dd:ee:ff"`
   - ❌ Ungültig: `"*"`, `"a*"`

3. **UUID:** Mindestens **8 zusammenhängende Hex-Zeichen**
   - ✅ Gültig: `"00003100-*"`, `"*-00805f9b"`
   - ❌ Ungültig: `"*"`, `"0000*"`

4. **Match-All Prevention:** Reine `*` oder `**` Patterns werden abgelehnt

### Startup-Validierung

Beim Boot werden alle Patterns automatisch validiert:

```
[PATTERN_CHECK] Starting pattern validation...
[PATTERN_CHECK] WiFi SSID: 6 valid, 0 invalid
[PATTERN_CHECK] MAC patterns: 20 valid, 0 invalid
[PATTERN_CHECK] BLE name patterns: 4 valid, 0 invalid
[PATTERN_CHECK] Raven UUID patterns: 8 valid, 0 invalid
[PATTERN_CHECK] Pattern validation complete
```

**Ungültige Patterns:**
- Werden beim Boot geloggt mit Fehlerbeschreibung
- Werden zur Laufzeit übersprungen (Graceful Degradation)
- Optional: Boot-Abort bei ungültigen Patterns (siehe Build-Flags)

### Build-Flags (Optional)

In `platformio.ini`:

```ini
build_flags = 
    -DENABLE_WILDCARD_VALIDATION=1      ; Aktiviert Pattern-Validierung (Default)
    -DSTRICT_PATTERN_VALIDATION=1       ; Boot-Abort bei ungültigen Patterns (Debug)
    -DDISABLE_WILDCARDS=1               ; Deaktiviert Wildcards (Legacy-Mode)
```

### Migration von alten Patterns

**Alte Patterns (Substring-Matching):**
```c
"Flock"     // Matcht: Flock, Flock-123, MyFlock (überall im String)
```

**Neue Patterns (Wildcard-Matching):**
```c
"Flock"     // Matcht: nur exakt "Flock"
"Flock*"    // Matcht: Flock, Flock-123 (Präfix)
"*Flock*"   // Matcht: Flock, MyFlock, Flock-123 (überall)
```

**Tipp:** Um altes Verhalten zu erhalten, wandle `"Flock"` → `"*Flock*"` um.

---

## 7) Testen / Verifizieren

Empfohlenes Vorgehen:
1. Debug-Log: Für jedes Event die extrahierten Felder loggen (MAC-Präfix, SSID, BLE Name, Service UUIDs).
2. Feldtest: Ein bekanntes Testgerät pro Kategorie als „Golden Sample".
3. Nach Änderungen immer `pio run`.
