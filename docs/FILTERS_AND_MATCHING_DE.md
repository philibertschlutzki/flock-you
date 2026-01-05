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

## 6) Fortgeschritten: Wildcards nachrüsten (SSID + BLE Name)

Die aktuelle Implementierung nutzt Substring-Matching. Wenn stattdessen Wildcards (z.B. `RAVEN_*`, `*-Guest`) benötigt werden, kann das ohne große Refactorings nachgerüstet werden.

### Ziel
- Patterns sollen `*` (beliebige Länge) und `?` (genau 1 Zeichen) unterstützen.
- Bestehende Listen bleiben Strings, aber werden als Wildcard-Pattern interpretiert.

### Schritt 1: Wildcard-Matcher als Helper
Lege eine kleine Helper-Datei an, z.B. `include/wildcard_match.h`:
```cpp
#pragma once

static inline bool wildcard_match_ci(const char* pattern, const char* s) {
  if (!pattern || !s) return false;

  // case-insensitive Vergleich ohne Heap, embedded-tauglich
  auto lower = [](char c) -> char {
    return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
  };

  const char* star = nullptr;
  const char* ss = s;

  while (*s) {
    char pc = lower(*pattern);
    char sc = lower(*s);

    if (pc == '?' || pc == sc) { pattern++; s++; continue; }
    if (pc == '*') { star = pattern++; ss = s; continue; }
    if (star) { pattern = star + 1; s = ++ss; continue; }
    return false;
  }

  while (*pattern == '*') pattern++;
  return *pattern == '\0';
}
```

### Schritt 2: WiFi SSID Matching umstellen
In `src/wifi_sniffer.cpp` die Funktion `wifi_check_ssid_pattern()` von `strcasestr()` auf den Wildcard-Matcher umstellen:
- Vorher: „SSID enthält Pattern“
- Nachher: „SSID matcht Wildcard-Pattern“

Pseudocode:
```cpp
#include "wildcard_match.h"

bool wifi_check_ssid_pattern(const char* ssid) {
  if (!ssid) return false;
  for (int i = 0; i < (int)(sizeof(wifi_ssid_patterns)/sizeof(wifi_ssid_patterns[0])); i++) {
    if (wildcard_match_ci(wifi_ssid_patterns[i], ssid)) return true;
  }
  return false;
}
```

### Schritt 3: BLE Device Name Matching umstellen
Analog in `src/ble_scanner.cpp` die Funktion `ble_check_device_name_pattern()` auf `wildcard_match_ci()` umstellen.

### Schritt 4: Patterns anpassen
Danach können in `include/detection_patterns.h` echte Wildcards verwendet werden:
```c
static const char* wifi_ssid_patterns[] = {
  "RAVEN_*",
  "*-Guest"
};

static const char* device_name_patterns[] = {
  "Raven?",
  "RAVEN-*"
};
```

### Test-Hinweise
- Mindestens ein Pattern ohne Wildcard behalten (Regression-Check).
- Für Debug: SSID/Name + matchendes Pattern mitloggen.

---

## 7) Testen / Verifizieren

Empfohlenes Vorgehen:
1. Debug-Log: Für jedes Event die extrahierten Felder loggen (MAC-Präfix, SSID, BLE Name, Service UUIDs).
2. Feldtest: Ein bekanntes Testgerät pro Kategorie als „Golden Sample“.
3. Nach Änderungen immer `pio run`.
