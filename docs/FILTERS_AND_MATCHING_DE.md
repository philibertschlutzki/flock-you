# Blacklist, Filter & Matching (WiFi + BLE)

Diese Anleitung beschreibt die **Blacklist-basierte Unknown-Detection** des Projekts.
Ziel: Der Scanner bleibt stabil, und das Verhalten wird über Daten (Blacklist) + optionale Session-Filter gesteuert.

## Überblick

Die Firmware arbeitet nach dem Prinzip:

- **Known** (in Blacklist / Session Filter) → wird gefiltert (keine Ausgabe)
- **Unknown** (nicht in Blacklist / Session Filter) → wird als Detection ausgegeben

Kern-API:

- `is_known_wifi(ssid, mac_a, mac_b)`
- `is_known_ble(mac)`

## 1) Datenquellen der Filterung

### 1.1 Compile-time Blacklist (generiert)

Die „Known“-Daten werden aus CSV-Dateien in `datasets/` generiert.
Der Generator schreibt (und überschreibt) diese Dateien:

- `include/known_blacklist_generated.h`
- `src/known_blacklist_generated.cpp`

Enthalten sind typischerweise:

- WiFi SSIDs (String)
- WiFi MACs (6-Byte)
- BLE MACs (6-Byte)
- OUIs (3-Byte Prefix)

### 1.2 Session Filter (RAM-only)

Zusätzlich gibt es einen Laufzeit-Filter (`session_filter.*`) als Hook für spätere WebGUI/API.

- Reset bei Reboot (nicht persistent)
- Ergänzt die „Known“-Prüfung zusätzlich zur generierten Blacklist

API (Auszug):

```c
void session_filter_clear();

bool session_filter_add_ssid(const char* ssid);
bool session_filter_add_mac_wifi(const uint8_t mac[6]);
bool session_filter_add_mac_ble(const uint8_t mac[6]);
bool session_filter_add_oui(const uint8_t oui[3]);
```

## 2) Matching-Regeln

### 2.1 SSID Matching

- **Exakter Match, case-sensitiv** (kein Wildcard/Pattern-Matching)
- Bei hidden/empty SSID wird SSID ignoriert und es wird nur über MAC/OUI entschieden

### 2.2 MAC Matching

- **Exakter 6-Byte Vergleich** gegen die jeweiligen Listen (WiFi/BLE)
- Zusätzlich wird die OUI geprüft (**erste 3 Bytes**)

### 2.3 WiFi: zwei MACs

Bei WiFi werden (je nach Frame-Typ) zwei MACs geprüft:

- `mac_a` (typisch Source/Client)
- `mac_b` (typisch BSSID)

Wenn eine der MACs (oder deren OUI) in der Blacklist/Session Filter ist, gilt das Gerät als „known“.

## 3) Detection-Ausgabe (Unknown-only)

Wenn ein Gerät **nicht** als „known“ erkannt wird, wird es ausgegeben.

- WiFi: `probe_request_unknown`, `beacon_unknown`
- BLE: `unknown`

Hinweis: Beim BLE-Scan kann ein Gerätename in der Ausgabe enthalten sein, er wird aber nicht zur Filterentscheidung verwendet.

## 4) Blacklist generieren

```bash
python3 tools/generate_blacklist_from_csv.py
```

Wenn `datasets/` geändert wurde, muss die Generierung erneut laufen, bevor ein Build zuverlässig funktioniert.

## 5) Testen / Verifizieren

Empfohlenes Vorgehen:

1. Einen „Known“-Eintrag (SSID oder MAC/OUI) in den CSV-Daten sicherstellen.
2. Blacklist generieren.
3. Firmware bauen/flashen.
4. Prüfen, dass bekannte Geräte **nicht** mehr als Unknown erscheinen, während neue/unbekannte Geräte als Unknown gemeldet werden.
