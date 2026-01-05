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

## 4) CSV-Dateiformat für Blacklist

Die CSV-Dateien im Verzeichnis `datasets/` müssen einem bestimmten Format entsprechen, damit der Generator sie korrekt verarbeiten kann.

### 4.1 Erforderliche Spalten

Die CSV-Dateien **müssen** folgende Spalten enthalten (Header-Zeile):

- `type` - Gerätetyp zur Unterscheidung zwischen WiFi und BLE
- `netid` - MAC-Adresse des Geräts (Format: `aa:bb:cc:dd:ee:ff` oder `aa-bb-cc-dd-ee-ff`)
- `ssid` - (Optional) WiFi SSID (nur für WiFi-Geräte relevant)

### 4.2 Weitere empfohlene Spalten

Für Kompatibilität mit Wardriving-Datenformaten werden häufig folgende zusätzliche Spalten verwendet:

- `trilat`, `trilong` - GPS-Koordinaten
- `qos`, `transid`, `firsttime`, `lasttime`, `lastupdt` - Metadaten
- `capabilities`, `userfound`, `device`, `mfgrId`, `name` - Geräteinformationen
- `country`, `region`, `road`, `city`, `housenumber`, `postalcode` - Standortinformationen

Diese zusätzlichen Spalten werden vom Generator **ignoriert** und sind optional.

### 4.3 Werte in der `type`-Spalte

- **WiFi-Geräte**: `infra` (oder leer/andere Werte außer `BLE`/`BT`)
- **BLE-Geräte**: `BLE` oder `BT`

### 4.4 Format der MAC-Adresse (`netid`)

- **Format**: 6 Bytes als Hex-Werte, getrennt durch `:` oder `-`
- **Beispiele**: 
  - `aa:bb:cc:dd:ee:ff`
  - `AA-BB-CC-DD-EE-FF` (Groß-/Kleinschreibung spielt keine Rolle)
- **Wichtig**: Ungültige MAC-Adressen werden vom Generator übersprungen und erzeugen eine Warnung

### 4.5 Format der SSID

- **Kodierung**: UTF-8
- **Matching**: Exakter Match, case-sensitiv
- **Leer/Hidden**: Leere SSIDs werden ignoriert; Filterung erfolgt nur über MAC/OUI
- **Sonderzeichen**: Werden automatisch escaped (z.B. `"` wird zu `\"`)

### 4.6 Beispiel-CSV (example_blacklist.csv)

Eine Beispiel-Blacklist-Datei befindet sich in `datasets/example_blacklist.csv`:

```csv
trilat,trilong,ssid,qos,transid,firsttime,lasttime,lastupdt,netid,type,capabilities,userfound,device,mfgrId,name,country,region,road,city,housenumber,postalcode
41.0925450,-81.5574720,MyHomeWiFi,0,20240101-00001,2024-01-01T10:00:00.000Z,2024-01-01T12:00:00.000Z,2024-01-01T12:00:00.000Z,aa:bb:cc:dd:ee:ff,infra,['WPA2'],False,0,,,US,OH,Main Street,Akron,123,44301
41.0930650,-81.5562140,OfficeNetwork,0,20240101-00002,2024-01-01T10:00:00.000Z,2024-01-01T12:00:00.000Z,2024-01-01T12:00:00.000Z,11:22:33:44:55:66,infra,['WPA2'],False,0,,,US,OH,Broadway Street,Akron,456,44302
41.0925740,-81.5463800,,0,20240101-00003,2024-01-01T10:00:00.000Z,2024-01-01T12:00:00.000Z,2024-01-01T12:00:00.000Z,aa:bb:cc:11:22:33,infra,[],False,0,,,US,OH,Market Street,Akron,789,44303
41.0923640,-81.5549830,MyDevice,0,20240102-00001,2024-01-02T08:00:00.000Z,2024-01-02T10:00:00.000Z,2024-01-02T10:00:00.000Z,de:ad:be:ef:ca:fe,BLE,['Misc'],False,7936,,MyDevice,US,OH,Exchange Street,Akron,321,44304
41.0918700,-81.5562540,SmartWatch,0,20240102-00002,2024-01-02T08:00:00.000Z,2024-01-02T10:00:00.000Z,2024-01-02T10:00:00.000Z,ff:ee:dd:cc:bb:aa,BLE,['Misc'],False,0,,SmartWatch,US,OH,Copley Road,Akron,654,44305
```

Diese Beispieldatei enthält:
- 2 WiFi-Einträge mit SSID (MyHomeWiFi, OfficeNetwork)
- 1 WiFi-Eintrag ohne SSID (hidden network)
- 2 BLE-Einträge (MyDevice, SmartWatch)

### 4.7 Verarbeitungslogik des Generators

Der Generator (`tools/generate_blacklist_from_csv.py`) verarbeitet alle `.csv`-Dateien in `datasets/`:

1. **Zeile für Zeile einlesen**: Jede Zeile wird als Gerät interpretiert
2. **Typ ermitteln**: 
   - `type == "BLE"` oder `type == "BT"` → BLE-Gerät
   - Sonst → WiFi-Gerät
3. **Daten extrahieren**:
   - **WiFi**: SSID + MAC-Adresse → in `known_wifi_ssids[]` und `known_wifi_macs[][]`
   - **BLE**: MAC-Adresse → in `known_ble_macs[][]`
   - **OUI**: Erste 3 Bytes jeder MAC → in `known_ouis[][]` (für WiFi und BLE)
4. **Duplikate entfernen**: Mehrfach vorkommende Einträge werden automatisch dedupliziert
5. **Ausgabe generieren**: 
   - `include/known_blacklist_generated.h`
   - `src/known_blacklist_generated.cpp`

### 4.8 Wichtige Hinweise

- **CSV-Encoding**: Dateien sollten UTF-8 kodiert sein
- **Fehlerbehandlung**: Ungültige Zeilen werden mit Warnung übersprungen
- **Leere Felder**: Leere `ssid` oder `netid` Felder sind zulässig und werden ignoriert
- **Minimales Format**: Theoretisch reicht eine CSV mit nur `type,netid,ssid` als Header

## 5) Blacklist generieren

```bash
python3 tools/generate_blacklist_from_csv.py
```

Wenn `datasets/` geändert wurde, muss die Generierung erneut laufen, bevor ein Build zuverlässig funktioniert.

## 6) Testen / Verifizieren

Empfohlenes Vorgehen:

1. Einen „Known“-Eintrag (SSID oder MAC/OUI) in den CSV-Daten sicherstellen.
2. Blacklist generieren.
3. Firmware bauen/flashen.
4. Prüfen, dass bekannte Geräte **nicht** mehr als Unknown erscheinen, während neue/unbekannte Geräte als Unknown gemeldet werden.
