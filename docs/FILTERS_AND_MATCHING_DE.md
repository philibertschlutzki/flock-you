# Filter & Matching (WiFi + BLE)

Diese Anleitung beschreibt, wie die Erkennungslogik für WiFi- und BLE-Events über Listen/Regeln angepasst werden kann.
Ziel: Regeln sind **datengetrieben** (Listen), und die Scan-Logik bleibt stabil.

> Empfehlung: Die Regeln als C++-Header (compile-time) pflegen, weil das auf Embedded i.d.R. am robustesten ist (kein Dateisystem nötig, keine Parser-Abhängigkeiten).

---

## 1) Grundprinzip: Datengetriebene Regeln

### Option A (empfohlen): Compile-time Konfiguration (Header)
1. Lege eine Datei an: `include/user_rules.h`
2. Dort werden Listen gepflegt (MAC-Präfixe, SSID-Pattern, BLE-Namen, Service-UUIDs).
3. Die Scan-/Parser-Module inkludieren diese Datei und nutzen die Regeln.

**Beispiel: `include/user_rules.h`**
```cpp
#pragma once
#include <cstddef>
#include <cstdint>

// -----------------------------
// 1) MAC-Präfixe / OUIs (3 Bytes)
// -----------------------------
static const uint8_t KNOWN_OUIS[][3] = {
  {0xFC, 0xFB, 0xFB}, // Beispiel
  {0xAC, 0xCF, 0x23}, // Beispiel
};
static constexpr size_t KNOWN_OUIS_COUNT = sizeof(KNOWN_OUIS) / sizeof(KNOWN_OUIS[0]);

// -----------------------------
// 2) SSID-Patterns (Wildcards)
// - '*' = beliebige Länge
// - '?' = genau 1 Zeichen
// -----------------------------
static const char* SSID_PATTERNS[] = {
  "RAVEN_*",
  "MyHomeWiFi",
  "*-Guest",
};
static constexpr size_t SSID_PATTERNS_COUNT = sizeof(SSID_PATTERNS) / sizeof(SSID_PATTERNS[0]);

// -----------------------------
// 3) BLE Device Name-Patterns
// (z.B. Prefix/Substring-Listen)
// -----------------------------
static const char* BLE_NAME_PATTERNS[] = {
  "Raven",
  "RAVEN-",
  "ShotSense", // Beispiel
};
static constexpr size_t BLE_NAME_PATTERNS_COUNT = sizeof(BLE_NAME_PATTERNS) / sizeof(BLE_NAME_PATTERNS[0]);

// -----------------------------
// 4) BLE Service UUIDs (Raven-Erkennung)
// Unterstützt 16-bit und 128-bit UUIDs
// -----------------------------

// 16-bit UUIDs (Bluetooth SIG short UUIDs)
static const uint16_t RAVEN_SERVICE_UUID16[] = {
  0x180D, // Beispiel (Heart Rate) -> ersetzen durch Raven-UUIDs
};
static constexpr size_t RAVEN_SERVICE_UUID16_COUNT = sizeof(RAVEN_SERVICE_UUID16) / sizeof(RAVEN_SERVICE_UUID16[0]);

// 128-bit UUIDs (Little Endian Byte-Order, wie in Advertising-Daten üblich)
static const uint8_t RAVEN_SERVICE_UUID128[][16] = {
  // Beispiel: 00112233-4455-6677-8899-aabbccddeeff
  {0xff,0xee,0xdd,0xcc,0xbb,0xaa,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00},
};
static constexpr size_t RAVEN_SERVICE_UUID128_COUNT = sizeof(RAVEN_SERVICE_UUID128) / sizeof(RAVEN_SERVICE_UUID128[0]);
```

### Option B: Runtime Konfiguration (JSON/CSV im Flash)
Wenn das Projekt bereits SPIFFS/LittleFS nutzt, können Listen als Datei gepflegt werden (z.B. `datasets/*.csv`).
Dann braucht es:
- Parser + Validierung
- Fallback-Regeln bei fehlender Datei
- Versionierung des Formats

---

## 2) MAC-Adress-Filterung (OUI / Präfix-Matching)

### Ziel
Geräte anhand bekannter Hersteller-Präfixe erkennen (OUI = erste 24 Bit / 3 Bytes einer MAC).

### Matching-Regel
1. Aus beobachteter MAC die ersten drei Bytes extrahieren.
2. Gegen `KNOWN_OUIS` vergleichen.
3. Treffer => Gerät als „bekannt“ markieren (oder priorisieren, labeln, etc.).

**Beispiel-Funktion (OUI-Match):**
```cpp
#include "user_rules.h"

static bool is_known_oui(const uint8_t mac[6]) {
  for (size_t i = 0; i < KNOWN_OUIS_COUNT; i++) {
    if (mac[0] == KNOWN_OUIS[i][0] &&
        mac[1] == KNOWN_OUIS[i][1] &&
        mac[2] == KNOWN_OUIS[i][2]) {
      return true;
    }
  }
  return false;
}
```

### Pflege der Liste
- Neue OUI hinzufügen, wenn ein relevantes Gerät in Logs auftaucht.
- OUIs in Hex pflegen (3 Bytes), ohne „:“-Schreibweise.

---

## 3) SSID-Muster-Abgleich (WiFi)

### Ziel
Access-Points/Clients/Beacons anhand SSID-Namen identifizieren.

### Empfohlenes Pattern-System
- `*` matcht beliebige Länge (inkl. leer)
- `?` matcht genau 1 Zeichen
- Alles andere ist Literal-Match

**Wildcard-Matcher (einfach, embedded-tauglich):**
```cpp
static bool wildcard_match(const char* pattern, const char* s) {
  const char* star = nullptr;
  const char* ss = s;

  while (*s) {
    if (*pattern == '?' || *pattern == *s) { pattern++; s++; continue; }
    if (*pattern == '*') { star = pattern++; ss = s; continue; }
    if (star) { pattern = star + 1; s = ++ss; continue; }
    return false;
  }
  while (*pattern == '*') pattern++;
  return *pattern == '\0';
}

static bool ssid_matches_any(const char* ssid) {
  for (size_t i = 0; i < SSID_PATTERNS_COUNT; i++) {
    if (wildcard_match(SSID_PATTERNS[i], ssid)) return true;
  }
  return false;
}
```

### Praxis-Tipps
- Eher wenige, klare Patterns statt vieler spezieller.
- Sonderfälle: versteckte SSIDs (leer), ungültige/kaputte Strings -> defensiv behandeln.

---

## 4) BLE Gerätenamen-Erkennung (Advertising Name)

### Ziel
BLE-Geräte anhand des beworbenen Namens (Complete Local Name / Shortened Local Name) erkennen.

### Matching-Strategien
- Prefix-Match (schnell, robust): `startsWith("Raven")`
- Substring-Match (flexibel): enthält `"Raven"` irgendwo
- Wildcards analog SSID (optional)

**Beispiel (Substring):**
```cpp
#include <cstring>
#include "user_rules.h"

static bool ble_name_matches_any(const char* name) {
  if (!name || !*name) return false;
  for (size_t i = 0; i < BLE_NAME_PATTERNS_COUNT; i++) {
    if (std::strstr(name, BLE_NAME_PATTERNS[i]) != nullptr) return true;
  }
  return false;
}
```

### Hinweis
Viele Geräte senden den Namen nicht in jedem Advertisement (oder nur „shortened“).
Deshalb: Name-Match als **Signal**, nicht als alleinige Wahrheit behandeln.

---

## 5) BLE Service UUID Erkennung (Raven Shot Detection)

### Ziel
Raven-Schusserkennungsgeräte über Service-UUID(s) in Advertising-Daten identifizieren.

### Datenquellen in BLE Advertising
- „Complete List of 16-bit Service UUIDs“ (AD Type 0x03)
- „Incomplete List of 16-bit Service UUIDs“ (AD Type 0x02)
- „Complete List of 128-bit Service UUIDs“ (0x07)
- „Incomplete List of 128-bit Service UUIDs“ (0x06)

### Matching-Logik
1. Advertising Payload parsen (TLV-Struktur).
2. UUID-Listen extrahieren (16-bit und/oder 128-bit).
3. Gegen `RAVEN_SERVICE_UUID16` und `RAVEN_SERVICE_UUID128` vergleichen.
4. Treffer => Gerät als „Raven“ markieren (hohe Priorität).

**Beispiel: 16-bit UUID Lookup**
```cpp
#include "user_rules.h"

static bool uuid16_is_raven(uint16_t uuid) {
  for (size_t i = 0; i < RAVEN_SERVICE_UUID16_COUNT; i++) {
    if (uuid == RAVEN_SERVICE_UUID16[i]) return true;
  }
  return false;
}
```

**Beispiel: 128-bit UUID Lookup**
```cpp
#include <cstring>
#include "user_rules.h"

static bool uuid128_is_raven(const uint8_t u[16]) {
  for (size_t i = 0; i < RAVEN_SERVICE_UUID128_COUNT; i++) {
    if (std::memcmp(u, RAVEN_SERVICE_UUID128[i], 16) == 0) return true;
  }
  return false;
}
```

### Wichtig: Byte-Order
BLE Advertising liefert 128-bit UUIDs typischerweise „little endian“ (byte-reversed gegenüber der üblichen UUID-String-Schreibweise).
Beim Eintragen der Werte unbedingt die gleiche Byte-Reihenfolge verwenden wie der Parser liefert.

---

## 6) Testen / Verifizieren

Empfohlenes Vorgehen:
1. Debug-Log: Für jedes Event die extrahierten Felder loggen (MAC/OUI, SSID, BLE Name, Service UUIDs).
2. Unit-Test / Sim-Test: Parser-Funktionen mit gespeicherten Beispiel-Frames füttern.
3. Feldtest: Ein bekanntes Testgerät pro Kategorie als „Golden Sample“.

---

## 7) Checkliste: Regeln erweitern

- [ ] Neues Gerät entdeckt → OUI ergänzen?
- [ ] Neues WLAN → SSID-Pattern ergänzen?
- [ ] Neues BLE-Gerät → Name-Pattern ergänzen?
- [ ] Raven-Gerät → Service-UUID ergänzen (16/128)?
- [ ] Danach: Build + Smoke-Test (Scan läuft, keine Abstürze)
