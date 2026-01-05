# Filter & Matching (WiFi + BLE)

Diese Anleitung beschreibt, wie die Erkennungslogik für WiFi- und BLE-Events über Listen/Regeln angepasst werden kann.
Ziel: Regeln sind **datengetrieben** (Listen), und die Scan-Logik bleibt stabil.

> Stand im Code (main): Die Filter nutzen **Wildcard-Matching** (case-insensitiv).
> Patterns ohne `*` sind **exakt**, und das frühere "Substring-Verhalten" erhält man durch `*pattern*`.

---

## 1) Schnellstart (Anfänger)

### Wo wird aktuell konfiguriert?
Die Filterlisten sind im Projekt zentral in `include/detection_patterns.h` abgelegt:
- `wifi_ssid_patterns[]` (WiFi SSID)
- `mac_prefixes[]` (MAC-Patterns, z.B. OUI als Präfix `aa:bb:cc:*`)
- `device_name_patterns[]` (BLE Name)
- `raven_service_uuids[]` + `RAVEN_*_SERVICE` (Raven Service UUIDs)

Für die meisten Anpassungen reicht es, **nur diese Arrays** zu erweitern.

### Änderungen in 3 Schritten
1. `include/detection_patterns.h` öffnen.
2. Passende Liste erweitern (siehe unten).
3. Build prüfen: `pio run`.

---

## 2) MAC-Adress-Filterung (Patterns)

### Was wird verglichen?
MAC-Adressen werden als String im Format `aa:bb:cc:dd:ee:ff` gegen die Liste `mac_prefixes[]` geprüft.
Dabei sind Wildcards erlaubt (z.B. `aa:bb:cc:*` für einen klassischen OUI-Präfix, oder `*:dd:ee:ff` für ein Suffix).

### Liste anpassen (Anfänger)
In `include/detection_patterns.h` unter `mac_prefixes[]` neue Einträge hinzufügen:

```c
static const char* mac_prefixes[] = {
    "58:8e:81:*",   // klassischer OUI-Präfix (erste 3 Bytes)
    "e4:aa:ea:*",
    "aa:bb:*"       // breiterer Präfix-Test
};
```

### Häufige Fehler
- Falsches Format: am besten immer Byteweise mit Doppelpunkten schreiben (`aa:bb:...`).
- Reines `"*"`: wird (je nach Validierung) als "match-all" betrachtet und sollte für Tests nur bewusst eingesetzt werden.

---

## 3) SSID-Muster-Abgleich (WiFi)

### Aktueller Stand
Im WiFi-Sniffer wird die SSID case-insensitiv gegen `wifi_ssid_patterns[]` gematcht.
Ohne Wildcard ist ein Pattern **exakt**; für "enthält"-Matching nutze `*pattern*`.

### Liste anpassen (Anfänger)
In `include/detection_patterns.h` unter `wifi_ssid_patterns[]` erweitern:

```c
static const char* wifi_ssid_patterns[] = {
    "*flock*",
    "*Pigvision*",
    "*MyTestSSID*" // neu
};
```

---

## 4) BLE Gerätenamen-Erkennung

### Aktueller Stand
Im BLE-Scanner wird der beworbene Gerätename case-insensitiv gegen `device_name_patterns[]` gematcht.
Ohne Wildcard ist ein Pattern **exakt**; für "enthält"-Matching nutze `*pattern*`.

### Liste anpassen (Anfänger)
In `include/detection_patterns.h` unter `device_name_patterns[]` erweitern:

```c
static const char* device_name_patterns[] = {
    "*Flock*",
    "*Penguin*",
    "*MyBLEDevice*" // neu
};
```

---

## 5) BLE Service UUID Erkennung (Raven)

### Aktueller Stand
Raven-Geräte werden über Service-UUIDs erkannt:
- Der Scanner liest alle beworbenen Service UUIDs.
- Jede UUID wird zu einem String normalisiert.
- Dann wird case-insensitiv gegen `raven_service_uuids[]` gematcht.

Ohne Wildcard ist der Vergleich **exakt**; mit Wildcard kannst du UUID-Familien abdecken (z.B. `00003100-*`).

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

## 6) Wildcard-Syntax

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
- `"*-Guest"` - Alle Gast-Netzwerke (endet auf "Guest")
- `"Test*Device"` - Testgeräte mit variablem Mittelteil
- `"*FLOCK*"` - Beliebiger Text mit "FLOCK" darin

### Wildcard-Beispiele für MAC-Adressen

```c
static const char* mac_prefixes[] = {
    "aa:bb:cc:*",     // Matcht: aa:bb:cc:dd:ee:ff
    "58:8e:*",        // Matcht: alle 58:8e:xx:xx:xx:xx
    "*:dd:ee:ff",     // Matcht: alle MACs mit Suffix dd:ee:ff
    "aa:*:ff"         // Matcht je nach Pattern-Engine mehrere Formen
};
```

### Wildcard-Beispiele für Service UUIDs (Raven)

```c
static const char* raven_service_uuids[] = {
    "00003100-*",                                    // Matcht: alle GPS Service Variants
    "*-00805f9b34fb",                                // Matcht: alle mit diesem Suffix
    "00003100-*-00805f9b34fb",                       // Präfix+Suffix
    "00003100-0000-1000-8000-00805f9b34fb"           // Exakt Match (kein Wildcard)
};
```

---

## 7) Pattern-Validierung (Boot)

Beim Boot können Patterns validiert werden (z.B. Mindest-Spezifität, match-all Prevention).
Welche Checks aktiv sind, kann über Build-Flags gesteuert werden (siehe `platformio.ini`).

**Wichtig:** Das Projekt kann die Validierung auch bewusst im Code/Headers deaktivieren (z.B. um kurze Test-Wildcards ohne Warnungen zu erlauben).

---

## 8) Testen / Verifizieren

Empfohlenes Vorgehen:
1. Debug-Log: Für jedes Event die extrahierten Felder loggen (SSID, MAC, BLE Name, Service UUIDs).
2. Feldtest: Ein bekanntes Testgerät pro Kategorie als „Golden Sample“.
3. Nach Änderungen immer `pio run`.
