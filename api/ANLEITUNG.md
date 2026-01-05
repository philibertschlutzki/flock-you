# Flock You API - Detaillierte Anleitung

## Übersicht

Die Flock You API ist ein Flask-basiertes Web-Dashboard zur Echtzeit-Überwachung und Analyse von Flock Safety Geräte-Erkennungen mit GPS-Integration. Das System ermöglicht die Erkennung von Überwachungsgeräten über WiFi- und Bluetooth-Protokolle mit präziser GPS-Ortung.

## Architektur und Komponenten

### Hauptkomponenten

1. **Flask Web-Server** (`flockyou.py`)
   - REST-API Endpoints für Gerätekommunikation
   - WebSocket-basierte Echtzeit-Updates (Flask-SocketIO)
   - Multi-Threading für parallele Geräteverbindungen
   - Persistente Datenspeicherung (Pickle-Format)

2. **Serial Port Kommunikation**
   - ESP32 Flock You Device (115200 Baud)
   - GPS-Dongle (9600 Baud, NMEA-Protokoll)
   - Automatische Wiederverbindung bei Verbindungsabbruch

3. **GPS-Integration**
   - NMEA GPGGA/GNGGA Sentence Parsing
   - Temporales GPS-Matching (±30 Sekunden)
   - Satellitenanzahl und Fix-Qualität
   - GPS-History Buffer (letzte 100 Positionen)

4. **Datenexport**
   - CSV-Export mit vollständigen Metadaten
   - KML-Export für Google Earth
   - Session- und kumulative Daten

5. **OUI-Datenbank**
   - IEEE MAC-Adress Hersteller-Lookup
   - Automatische Herstellererkennung
   - Aktualisierbare Datenbank

## Installation und Setup

### Systemvoraussetzungen

```bash
# Python 3.8 oder höher
python --version

# Empfohlenes Betriebssystem: Linux (Ubuntu/Debian) oder macOS
```

### Abhängigkeiten installieren

```bash
cd api/
pip install -r requirements.txt
```

**requirements.txt Inhalt:**
- Flask: Web-Framework
- Flask-SocketIO: WebSocket-Support
- python-socketio: Socket.IO Client/Server
- pyserial: Serial Port Kommunikation

### Server starten

```bash
python flockyou.py
```

Der Server startet auf `http://0.0.0.0:5000` und ist im lokalen Netzwerk erreichbar.

## Funktionsweise

### 1. Geräteverbindung

#### ESP32 Flock You Device verbinden

1. **Device-Erkennung:**
   - API-Endpoint: `GET /api/flock/ports`
   - Listet alle verfügbaren Serial Ports
   - Zeigt Hersteller, Produkt, VID/PID

2. **Verbindung herstellen:**
   ```bash
   POST /api/flock/connect
   Content-Type: application/json
   
   {
     "port": "/dev/ttyUSB0"
   }
   ```

3. **Hintergrund-Thread:**
   - Startet automatisch `flock_reader()` Thread
   - Liest kontinuierlich Serial-Daten
   - Parst JSON-Detektionen
   - Buffer der letzten 1000 Zeilen für Terminal

#### GPS-Dongle verbinden

1. **GPS-Ports abrufen:**
   ```bash
   GET /api/gps/ports
   ```

2. **GPS verbinden:**
   ```bash
   POST /api/gps/connect
   Content-Type: application/json
   
   {
     "port": "/dev/ttyACM0"
   }
   ```

3. **GPS-Thread:**
   - Parst NMEA-Sentences (GPGGA/GNGGA)
   - Konvertiert Koordinaten (DDMM.MMMM → Dezimalgrad)
   - 8 Dezimalstellen Präzision (~1.1mm Genauigkeit)
   - Speichert GPS-History für temporales Matching

### 2. Detektions-Verarbeitung

#### Datenfluss

```
ESP32 Device → Serial Port → JSON Parsing → Detection Processing → WebSocket Broadcast
                                    ↓
                              GPS Matching
                                    ↓
                            OUI Manufacturer Lookup
                                    ↓
                          Session + Cumulative Storage
```

#### Detection-Format (ESP32 → API)

```json
{
  "timestamp": 12345,
  "detection_time": "12.345s",
  "protocol": "wifi",
  "detection_method": "probe_request",
  "ssid": "Flock_Camera_001",
  "mac_address": "aa:bb:cc:dd:ee:ff",
  "rssi": -65,
  "signal_strength": "MEDIUM",
  "channel": 6
}
```

#### Detection Processing Steps

1. **Timestamp-Verarbeitung:**
   - System-Timestamp hinzufügen
   - GPS-Timestamp bevorzugen (wenn verfügbar)
   - `timestamp_source`: "gps" oder "system"

2. **GPS-Matching (Temporal):**
   - Suche in GPS-History (letzte 100 Einträge)
   - Max. ±30 Sekunden Zeitdifferenz
   - Match-Qualität: "temporal" oder "current"
   - Zeit-Differenz wird gespeichert

3. **OUI-Lookup:**
   - Extrahiert erste 6 Hex-Zeichen der MAC
   - Sucht in `oui.txt` Datenbank
   - Liefert Hersteller-Name

4. **Duplikat-Erkennung:**
   - Prüfung auf existierende MAC-Adresse
   - Inkrementiert `detection_count`
   - Aktualisiert `last_seen`, `last_rssi`, etc.

5. **Persistierung:**
   - Session-Detektionen (im RAM)
   - Kumulative Detektionen (`data/cumulative_detections.pkl`)
   - Automatisches Speichern nach jeder Detektion

### 3. GPS-Integration Details

#### NMEA Sentence Parsing

**Unterstützte Formate:**
- `$GPGGA`: GPS-only Fix
- `$GNGGA`: GPS + GLONASS Fix

**Parsing-Logik:**
```python
Format: $GPGGA,HHMMSS.SSS,DDMM.MMMM,N,DDDMM.MMMM,E,1,08,0.9,545.4,M,46.9,M,,*47
         Zeit      Latitude   Longitude  Fix Sats
```

**Koordinaten-Konversion:**
- NMEA: DDMM.MMMM (Grad + Minuten)
- Dezimal: DD.DDDDDD (Dezimalgrad)
- Formel: `Decimal = Degrees + (Minutes / 60.0)`
- Präzision: 8 Dezimalstellen

**GPS-Validierung:**
- Fix Quality ≥ 1
- Latitude: -90° bis +90°
- Longitude: -180° bis +180°
- HDOP (Horizontal Dilution of Precision)

#### Temporales GPS-Matching

**Funktionsweise:**
1. Detection erhält `server_timestamp` (System-Zeit)
2. Suche in `gps_history` nach zeitlich nächstem GPS-Fix
3. Zeitdifferenz muss ≤30 Sekunden sein
4. Speicherung der Zeitdifferenz und Match-Qualität

**Match-Qualitätsstufen:**
- **Präzise** (< 5s): ✓ GPS-Timestamp als Primär-Timestamp
- **Gut** (5-15s): ~ Verwendbar mit Zeitdifferenz-Angabe
- **Approximiert** (15-30s): ⚠ GPS-Position unsicher
- **Unbekannt**: ? Kein temporales Matching möglich

### 4. WebSocket Kommunikation

#### Echtzeit-Events

**Client → Server:**
```javascript
// Verbindung
socket.connect();

// Heartbeat (Keep-Alive)
socket.emit('heartbeat');

// Serial Terminal anfordern
socket.emit('request_serial_terminal', { port: '/dev/ttyUSB0' });
```

**Server → Client:**
```javascript
// Neue Detektion
socket.on('new_detection', (data) => { ... });

// Aktualisierte Detektion (erhöhter Count)
socket.on('detection_updated', (data) => { ... });

// GPS-Update
socket.on('gps_update', (data) => { ... });

// Serial Terminal Daten
socket.on('serial_data', (line) => { ... });

// Status-Events
socket.on('gps_disconnected', () => { ... });
socket.on('gps_reconnected', (data) => { ... });
socket.on('flock_disconnected', () => { ... });
socket.on('flock_reconnected', (data) => { ... });
```

### 5. Datenexport

#### CSV-Export

**Endpoint:**
```bash
GET /api/export/csv?type=session     # Session-Daten
GET /api/export/csv?type=cumulative  # Kumulative Daten
```

**CSV-Felder:**
- `timestamp`, `detection_time`, `server_timestamp`
- `protocol`, `detection_method`
- `ssid`, `device_name`, `mac_address`, `manufacturer`, `alias`
- `rssi`, `last_rssi`, `signal_strength`
- `channel`, `last_channel`, `detection_count`
- `latitude`, `longitude`, `altitude`
- `gps_timestamp`, `satellites`, `fix_quality`
- `gps_time_diff`, `gps_match_quality`, `timestamp_source`

#### KML-Export (Google Earth)

**Endpoint:**
```bash
GET /api/export/kml?type=session
GET /api/export/kml?type=cumulative
```

**KML-Features:**
- Placemarks für jede Detektion mit GPS
- GPS-Genauigkeits-Indikator (✓, ~, ⚠, ?)
- Vollständige Metadaten im Popup
- Alias-Support für benannte Geräte
- Höheninformation (Altitude)

### 6. Persistente Datenspeicherung

#### Dateien

```
api/
├── data/
│   ├── cumulative_detections.pkl  # Alle jemals erkannten Geräte
│   └── settings.json              # Benutzer-Einstellungen
├── exports/
│   ├── flockyou_session_*.csv
│   ├── flockyou_session_*.kml
│   └── flockyou_cumulative_*.csv
└── oui.txt                        # IEEE OUI Datenbank
```

#### Session vs. Kumulative Daten

**Session-Daten:**
- Im RAM (Variable `detections`)
- Wird bei Server-Neustart geleert
- Kann manuell gelöscht werden (`POST /api/clear`)
- Für aktuelle Überwachungs-Session

**Kumulative Daten:**
- Persistiert auf Festplatte (Pickle)
- Überlebt Server-Neustarts
- Historische Aufzeichnung aller Detektionen
- Automatische Synchronisation bei jeder Detektion

### 7. Verbindungsüberwachung

#### Connection Monitor Thread

**Funktionen:**
- Prüft alle 2 Sekunden Geräteverbindungen
- Erkennt unterbrochene Verbindungen
- Triggert automatische Wiederverbindung
- Max. 5 Reconnect-Versuche
- 3 Sekunden Wartezeit zwischen Versuchen

**Reconnect-Logik:**
```python
# Verbindung verloren → Detection
↓
Broadcast disconnect event
↓
Starte reconnect_thread
↓
Max 5 Versuche mit 3s Wartezeit
↓
Bei Erfolg: Neustart des Reader-Threads
Bei Fehler: reconnect_failed event
```

## API-Referenz

### Detection Management

#### GET /api/detections

**Beschreibung:** Abrufen aller Detektionen mit optionaler Filterung

**Query-Parameter:**
- `filter`: "all" | "probe_request" | "beacon" | "ble_advertisement" | etc.
- `type`: "session" | "cumulative"

**Response:**
```json
[
  {
    "id": 1,
    "timestamp": "2024-01-05T14:30:00",
    "detection_time": "14:30:00",
    "server_timestamp": "2024-01-05T14:30:00.123456",
    "protocol": "wifi",
    "detection_method": "probe_request",
    "mac_address": "aa:bb:cc:dd:ee:ff",
    "manufacturer": "Flock Group Inc.",
    "alias": "Kamera Hauptstraße",
    "rssi": -65,
    "signal_strength": "MEDIUM",
    "channel": 6,
    "detection_count": 5,
    "first_seen": "2024-01-05T14:25:00",
    "last_seen": "2024-01-05T14:30:00",
    "gps": {
      "latitude": 47.3769,
      "longitude": 8.5417,
      "altitude": 408.5,
      "satellites": 8,
      "fix_quality": 1,
      "time_diff": 2.3,
      "match_quality": "temporal"
    },
    "timestamp_source": "gps"
  }
]
```

#### POST /api/detections

**Beschreibung:** Neue Detektion vom ESP32 Device hinzufügen

**Request Body:**
```json
{
  "protocol": "wifi",
  "detection_method": "probe_request",
  "ssid": "Flock_Camera_001",
  "mac_address": "aa:bb:cc:dd:ee:ff",
  "rssi": -65,
  "signal_strength": "MEDIUM",
  "channel": 6
}
```

**Response:**
```json
{
  "status": "success",
  "id": 1
}
```

#### POST /api/clear

**Beschreibung:** Session-Detektionen löschen

**Response:**
```json
{
  "status": "success",
  "message": "Session detections cleared"
}
```

#### POST /api/detection/alias

**Beschreibung:** Alias für Detektion setzen/ändern

**Request Body:**
```json
{
  "id": 1,
  "alias": "Kamera Hauptstraße"
}
```

### GPS Management

#### GET /api/gps/ports

**Beschreibung:** Verfügbare Serial Ports für GPS auflisten

**Response:**
```json
[
  {
    "device": "/dev/ttyACM0",
    "description": "USB GPS Receiver",
    "manufacturer": "u-blox AG",
    "product": "u-blox 7",
    "vid": 5446,
    "pid": 423
  }
]
```

#### POST /api/gps/connect

**Beschreibung:** GPS-Dongle verbinden

**Request Body:**
```json
{
  "port": "/dev/ttyACM0"
}
```

#### POST /api/gps/disconnect

**Beschreibung:** GPS-Dongle trennen

### Flock Device Management

#### GET /api/flock/ports

**Beschreibung:** Verfügbare Serial Ports für ESP32 Device

#### POST /api/flock/connect

**Request Body:**
```json
{
  "port": "/dev/ttyUSB0"
}
```

#### POST /api/flock/disconnect

### Status & Statistics

#### GET /api/status

**Beschreibung:** Verbindungsstatus beider Geräte

**Response:**
```json
{
  "gps_connected": true,
  "gps_port": "/dev/ttyACM0",
  "flock_connected": true,
  "flock_port": "/dev/ttyUSB0"
}
```

#### GET /api/stats

**Beschreibung:** Detektions-Statistiken

**Response:**
```json
{
  "session": {
    "total": 42,
    "wifi": 35,
    "ble": 7,
    "gps": 40,
    "start_time": "2024-01-05T14:00:00"
  },
  "cumulative": {
    "total": 156,
    "wifi": 120,
    "ble": 36,
    "gps": 145
  }
}
```

### OUI Database

#### POST /api/oui/search

**Beschreibung:** OUI-Datenbank durchsuchen

**Request Body:**
```json
{
  "query": "Flock"
}
```

**Response:**
```json
{
  "status": "success",
  "results": [
    {
      "mac": "AABBCC",
      "manufacturer": "Flock Group Inc."
    }
  ],
  "count": 1
}
```

#### GET /api/oui/all

**Beschreibung:** Alle OUI-Einträge abrufen

#### POST /api/oui/refresh

**Beschreibung:** OUI-Datenbank von IEEE aktualisieren

**Warnung:** Kann HTTP 418 Error verursachen (IEEE Anti-Bot-Schutz)

### Settings

#### GET /api/settings

**Response:**
```json
{
  "gps_port": "/dev/ttyACM0",
  "flock_port": "/dev/ttyUSB0",
  "filter": "all"
}
```

#### POST /api/settings

**Request Body:**
```json
{
  "filter": "probe_request"
}
```

### Testing

#### POST /api/test/detection

**Beschreibung:** Test-Detektion hinzufügen (für Entwicklung)

**Optional Request Body:**
```json
{
  "detection_method": "probe_request",
  "mac_address": "aa:bb:cc:dd:ee:ff",
  "ssid": "TestNetwork"
}
```

## Erweiterte Konfiguration

### Umgebungsvariablen

```bash
# Flask Secret Key (für Production)
export SECRET_KEY="your-secure-random-key-here"

# Server Port (Standard: 5000)
export FLASK_PORT=8080

# Server Host (Standard: 0.0.0.0)
export FLASK_HOST="127.0.0.1"
```

### GPS-Konfiguration

**Konstanten in `flockyou.py`:**
```python
GPS_BAUDRATE = 9600              # GPS-Baudrate
GPS_TIMEOUT = 1                  # Read-Timeout
MAX_GPS_HISTORY = 100            # GPS-History Größe
GPS_MATCH_THRESHOLD = 30         # Max. Zeitdifferenz (Sekunden)
```

### Reconnection-Einstellungen

```python
max_reconnect_attempts = 5       # Max. Wiederverbindungs-Versuche
reconnect_delay = 3              # Wartezeit zwischen Versuchen
```

## Fehlerbehebung

### GPS-Verbindungsprobleme

**Problem:** GPS verbindet nicht

**Lösungen:**
```bash
# 1. Serial Port Berechtigungen prüfen
ls -l /dev/ttyACM0
sudo chmod 666 /dev/ttyACM0

# 2. Benutzer zur dialout-Gruppe hinzufügen
sudo usermod -a -G dialout $USER
# Neuanmeldung erforderlich

# 3. GPS-Dongle auf Satellitenempfang prüfen
cat /dev/ttyACM0
# Sollte NMEA-Sentences anzeigen

# 4. USB-Verbindung testen
lsusb
dmesg | grep tty
```

**Problem:** GPS Fix Quality = 0

**Ursachen:**
- Kein Satellitenempfang (Indoor-Nutzung)
- GPS-Dongle benötigt Kaltstart (5-10 Minuten)
- Abgeschirmte Umgebung
- Defektes GPS-Modul

**Lösung:** GPS-Dongle ans Fenster oder nach draußen positionieren

### ESP32 Device-Probleme

**Problem:** Device sendet keine Daten

**Debug-Schritte:**
```bash
# 1. Serial Terminal öffnen
screen /dev/ttyUSB0 115200
# oder
picocom /dev/ttyUSB0 -b 115200

# 2. Device-Ausgabe prüfen
# Erwartete JSON-Detektionen sollten sichtbar sein

# 3. Baudrate verifizieren (muss 115200 sein)

# 4. ESP32 Reset durchführen
# EN-Button oder USB-Kabel trennen/verbinden
```

### Datenexport-Probleme

**Problem:** Keine GPS-Koordinaten im Export

**Ursache:** Detektionen ohne GPS-Fix erfasst

**Lösung:**
1. GPS verbinden VOR dem Start der Detektionen
2. GPS Fix Quality im Dashboard prüfen
3. Temporales Matching funktioniert nur mit GPS-History

### Performance-Probleme

**Problem:** Hohe CPU-Last

**Optimierungen:**
```python
# Serial Read Delay erhöhen (in Reader-Threads)
time.sleep(0.1)  # → time.sleep(0.5)

# GPS History reduzieren
MAX_GPS_HISTORY = 100  # → 50

# WebSocket Broadcast optimieren
# Nur an Clients senden, die dashboard aktiv haben
```

**Problem:** Speicher-Leak bei langen Sessions

**Lösung:**
```python
# Serial Buffer limitieren (bereits implementiert)
if len(serial_data_buffer) > 1000:
    serial_data_buffer.pop(0)

# Session-Daten regelmäßig clearen
# POST /api/clear
```

## Best Practices

### 1. GPS-Nutzung

✅ **Empfohlen:**
- GPS vor Session-Start verbinden
- Satellitenanzahl ≥ 6 abwarten
- GPS-Antenne optimal positionieren
- GPS-History für präzises Matching nutzen

❌ **Vermeiden:**
- GPS während laufender Session trennen
- Indoor-Nutzung ohne externe Antenne
- GPS nachträglich verbinden (Detektionen haben kein GPS)

### 2. Session-Management

✅ **Empfohlen:**
- Session-Daten regelmäßig exportieren
- Kumulative Daten als Backup
- Aliase für bekannte Geräte setzen
- Filter für spezifische Detection-Methods nutzen

❌ **Vermeiden:**
- Server ohne Export-Backup neustarten
- Zu lange Sessions (>1000 Detektionen)
- Manuelles Löschen von `data/*.pkl`

### 3. Geräteverbindung

✅ **Empfohlen:**
- Connection Monitor aktiv lassen
- Auto-Reconnect nutzen
- Serial Ports in Settings speichern
- Mehrere USB-Hubs für Stabilität

❌ **Vermeiden:**
- Manuelle USB-Trennung während Betrieb
- Shared Serial Ports (andere Programme)
- Zu lange USB-Kabel (>3m)

### 4. Datenqualität

✅ **Empfohlen:**
- GPS temporal matching bevorzugen
- Timestamp source prüfen ("gps" vs "system")
- OUI-Datenbank aktuell halten
- RSSI-Werte für Distanz-Schätzung nutzen

❌ **Vermeiden:**
- Rein systembasierte Timestamps
- GPS-Daten ohne Validierung
- Alte OUI-Datenbank (>6 Monate)

## Sicherheitshinweise

### Netzwerk-Sicherheit

⚠️ **Wichtig:** Server läuft auf `0.0.0.0:5000` (öffentlich erreichbar)

**Empfohlene Maßnahmen:**

1. **Firewall konfigurieren:**
```bash
# Nur lokaler Zugriff
sudo ufw allow from 127.0.0.1 to any port 5000

# Oder spezifisches Netzwerk
sudo ufw allow from 192.168.1.0/24 to any port 5000
```

2. **Reverse Proxy (nginx):**
```nginx
server {
    listen 80;
    server_name flockyou.local;
    
    location / {
        proxy_pass http://127.0.0.1:5000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
    }
}
```

3. **HTTPS/TLS:**
- Let's Encrypt für öffentliche Domains
- Self-signed Certificate für lokale Nutzung

4. **Authentifizierung:**
- Flask-Login für Benutzer-Management
- API-Keys für ESP32-Zugriff
- Session-basierte Authentifizierung

### Daten-Sicherheit

**Sensible Daten:**
- GPS-Koordinaten (exakte Standorte)
- MAC-Adressen (Geräte-Identifikation)
- Timestamps (Bewegungsprofile)

**Schutzmaßnahmen:**
```bash
# 1. Dateiberechtigungen
chmod 600 data/*.pkl
chmod 700 exports/

# 2. Verschlüsselung
# Exports mit GPG verschlüsseln
gpg --encrypt --recipient your@email.com export.csv

# 3. Automatische Löschung
# Cron-Job für alte Exports
0 0 * * * find /path/to/exports -mtime +30 -delete
```

## Production Deployment

### Systemd Service

**Datei:** `/etc/systemd/system/flockyou.service`

```ini
[Unit]
Description=Flock You API Server
After=network.target

[Service]
Type=simple
User=flockyou
WorkingDirectory=/opt/flockyou/api
Environment="SECRET_KEY=your-production-key"
ExecStart=/usr/bin/python3 /opt/flockyou/api/flockyou.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

**Aktivierung:**
```bash
sudo systemctl daemon-reload
sudo systemctl enable flockyou
sudo systemctl start flockyou
sudo systemctl status flockyou
```

### Docker Deployment

**Dockerfile:**
```dockerfile
FROM python:3.11-slim

WORKDIR /app

# Install dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy application
COPY . .

# Create data directory
RUN mkdir -p data exports

# Expose port
EXPOSE 5000

# Run server
CMD ["python", "flockyou.py"]
```

**docker-compose.yml:**
```yaml
version: '3.8'

services:
  flockyou:
    build: .
    ports:
      - "5000:5000"
    devices:
      - "/dev/ttyUSB0:/dev/ttyUSB0"  # ESP32
      - "/dev/ttyACM0:/dev/ttyACM0"  # GPS
    volumes:
      - ./data:/app/data
      - ./exports:/app/exports
    environment:
      - SECRET_KEY=${SECRET_KEY}
    restart: unless-stopped
```

**Start:**
```bash
docker-compose up -d
docker-compose logs -f
```

## Monitoring und Logging

### Logging konfigurieren

```python
import logging

# Am Anfang von flockyou.py hinzufügen
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('flockyou.log'),
        logging.StreamHandler()
    ]
)

logger = logging.getLogger(__name__)
```

### Prometheus Metrics (Optional)

```python
from prometheus_flask_exporter import PrometheusMetrics

metrics = PrometheusMetrics(app)
metrics.info('flockyou_info', 'Flock You API', version='0.1.0')
```

**Metriken:**
- `flockyou_detections_total`: Gesamtanzahl Detektionen
- `flockyou_gps_fixes_total`: GPS-Fixes
- `flockyou_connection_errors_total`: Verbindungsfehler
- `flockyou_export_requests_total`: Export-Anfragen

## Erweiterungen und Customization

### Eigene Detection-Methods hinzufügen

**ESP32 Code:**
```cpp
JsonObject detection = doc.to<JsonObject>();
detection["detection_method"] = "custom_method";
detection["protocol"] = "custom";
// ... weitere Felder
```

**API Code (flockyou.py):**
```python
# Automatische Verarbeitung, keine Änderungen nötig
# Detection-Method wird dynamisch erkannt
```

### Webhook-Integration

```python
import requests

def send_webhook(detection):
    """Send detection to external webhook"""
    webhook_url = "https://your-server.com/webhook"
    try:
        requests.post(webhook_url, json=detection, timeout=5)
    except Exception as e:
        print(f"Webhook error: {e}")

# In add_detection_from_serial() einbauen:
add_detection_from_serial(data):
    # ... existing code ...
    send_webhook(data)
```

### Datenbank-Backend (SQLite)

```python
import sqlite3

def init_database():
    conn = sqlite3.connect('detections.db')
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS detections (
            id INTEGER PRIMARY KEY,
            timestamp TEXT,
            mac_address TEXT,
            protocol TEXT,
            detection_method TEXT,
            latitude REAL,
            longitude REAL,
            rssi INTEGER,
            detection_count INTEGER
        )
    ''')
    conn.commit()
    conn.close()
```

### Telegram Bot Integration

```python
import telegram

bot = telegram.Bot(token='YOUR_BOT_TOKEN')

def notify_telegram(detection):
    """Send alert to Telegram"""
    message = f"""
🚨 Neue Detektion
MAC: {detection['mac_address']}
Hersteller: {detection.get('manufacturer', 'Unknown')}
RSSI: {detection['rssi']} dBm
GPS: {detection.get('gps', {}).get('latitude')}, {detection.get('gps', {}).get('longitude')}
    """
    bot.send_message(chat_id='YOUR_CHAT_ID', text=message)
```

## Lizenz und Credits

**Projekt:** Flock You Detection System
**Version:** 0.1.0
**Lizenz:** [Lizenz des Hauptprojekts]

**Verwendete Bibliotheken:**
- Flask (BSD License)
- Flask-SocketIO (MIT License)
- pySerial (BSD License)
- Python Standard Library (PSF License)

**IEEE OUI Database:**
- © IEEE Registration Authority
- Standards-OUI: https://standards-oui.ieee.org/

## Weitere Ressourcen

**Dokumentation:**
- Flask Dokumentation: https://flask.palletsprojects.com/
- Socket.IO Dokumentation: https://socket.io/docs/
- pySerial Dokumentation: https://pyserial.readthedocs.io/
- NMEA Protocol: https://www.nmea.org/

**Tools:**
- Wireshark (Packet Analysis)
- Ghidra (Firmware Analysis)
- Google Earth (KML Visualization)
- QGIS (GIS Analysis)

**Community:**
- GitHub Issues: [Repository Issues]
- Discussions: [Repository Discussions]

---

**Erstellt:** 2025-01-05  
**Autor:** Automatisch generiert basierend auf Code-Analyse  
**Status:** Dokumentation vollständig