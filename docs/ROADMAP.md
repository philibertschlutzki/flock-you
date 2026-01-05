# Flock You – Roadmap

**Entwicklungsplan und Feature-Planung**

Diese Roadmap zeigt geplante Features und Verbesserungen für das Flock You Detection System. Die primäre Zielhardware ist das **LILYGO T-Display-S3** Board.

---

## 🎯 Versionierungsschema

- **v0.x**: Beta-Versionen, experimentelle Features, Breaking Changes möglich
- **v1.x**: Stabile Releases, rückwärtskompatibel innerhalb Major-Version
- **v2.x+**: Zukünftige Major-Releases mit neuen Hauptfeatures

---

## 📋 Kurzfristig (v0.1 - v0.2)

**Ziel:** Stabilisierung, Core-Features, Diagnostics

### v0.1.0 – Initial Release ✅ IN ARBEIT

**Features:**
- ✅ LILYGO T-Display-S3 als primäre Zielhardware
- ✅ PlatformIO Build-System mit `pio run` Support
- ✅ Modulare Code-Struktur (wifi_sniffer, ble_scanner, ui_display, diagnostics)
- ✅ Hardware Diagnostics Mode (Display, Backlight, Buttons, WiFi, BLE, Memory)
- ✅ Display UI mit Status und Alert-Screen
- ✅ WiFi Promiscuous Mode + Channel Hopping
- ✅ BLE Scanning (NimBLE) mit Raven UUID Detection
- ✅ JSON Serial Output für alle Detections
- ✅ Deutsche Dokumentation (README, Flashing Guide, Diagnostics)

**Nutzen:** Funktionierendes MVP mit vollständiger Hardware-Unterstützung

**Aufwand:** M (mittel, ca. 2-3 Wochen)

**Status:** 🟢 In Entwicklung

---

### v0.2.0 – Logging & Datenexport

**Features:**
- [ ] Persistent Logging auf SD-Karte (optional, falls SD-Slot vorhanden)
- [ ] CSV Export der Detection-Daten
- [ ] Filter/Whitelist Konfiguration (bestimmte MACs ignorieren)
- [ ] RSSI-Threshold Einstellungen (nur starke Signale)
- [ ] Erweiterte JSON-Schema-Dokumentation
- [ ] Timestamp mit RTC-Sync (NTP via WiFi)

**Nutzen:**
- Langzeit-Datenerfassung möglich
- Analysen und Reports für Nutzer
- Weniger False Positives durch Filter

**Aufwand:** M (mittel, ca. 1-2 Wochen)

**Risiken:**
- SD-Karte benötigt zusätzliche Pins (Konflikt mit Display?)
- NTP benötigt WiFi-Verbindung (Promiscuous Mode deaktivieren?)

**Abhängigkeiten:**
- SD-Card Library (falls Hardware-Support)
- NTP Client Library

---

## 🚀 Mittelfristig (v0.3 - v0.5)

**Ziel:** Web-UI Integration, Performance, Erweiterte Features

### v0.3.0 – Web-UI Integration

**Features:**
- [ ] Optimierung der Python API (`api/flockyou.py`)
- [ ] Live-Karte mit Detections (Leaflet/OpenStreetMap)
- [ ] Echtzeit-Graphen (RSSI über Zeit, Detections/Minute)
- [ ] Web-basierte Konfiguration (Filter, Thresholds)
- [ ] Export-Funktionen (CSV, KML, GeoJSON)
- [ ] Multi-Device Support (mehrere ESP32 parallel)

**Nutzen:**
- Professionelles Dashboard für Power-User
- Bessere Datenvisualisierung
- Flexiblere Konfiguration ohne Re-Flash

**Aufwand:** L (groß, ca. 3-4 Wochen)

**Risiken:**
- Komplexität der Web-Entwicklung
- WebSocket-Stabilität

**Abhängigkeiten:**
- Python Flask (bereits vorhanden)
- JavaScript Libraries (Leaflet, Chart.js)

---

### v0.4.0 – Performance & Optimierung

**Features:**
- [ ] Optimierter WiFi Channel Hopping Algorithmus
- [ ] Intelligentes BLE Scanning (adaptive Intervalle)
- [ ] PSRAM-Nutzung für größere Buffer
- [ ] Power Management (Deep Sleep zwischen Scans)
- [ ] Batterie-Betrieb Optimierung
- [ ] Speicher-Leak Tests und Fixes

**Nutzen:**
- Längere Akkulaufzeit (falls Batterie-betrieben)
- Mehr Detections pro Zeiteinheit
- Stabilität bei Langzeit-Betrieb

**Aufwand:** M (mittel, ca. 2 Wochen)

**Risiken:**
- Power Management kann komplexe Timing-Probleme verursachen
- Deep Sleep unterbricht Promiscuous Mode

---

### v0.5.0 – Pattern Expansion

**Features:**
- [ ] Erweiterte MAC-Prefix Database (mehr Hersteller)
- [ ] SSID Pattern Learning (Machine Learning optional)
- [ ] Raven Firmware Version Detection erweitern
- [ ] Custom Pattern Editor (Web-UI)
- [ ] Community-Patterns Import/Export
- [ ] Confidence Scoring Verbesserung

**Nutzen:**
- Höhere Detection-Rate
- Weniger False Negatives
- Community-getriebene Verbesserungen

**Aufwand:** M (mittel, ca. 2-3 Wochen)

**Risiken:**
- ML benötigt viel Rechenleistung (ESP32 limitiert)
- Community-Patterns Qualitätssicherung

---

## 🌟 Langfristig (v1.0+)

**Ziel:** Produktionsreife, OTA Updates, Plugin-System

### v1.0.0 – Production Ready

**Features:**
- [ ] Vollständige Unit-Test-Abdeckung (ESP32 + Python)
- [ ] CI/CD Pipeline (GitHub Actions)
- [ ] Automatische Release-Builds
- [ ] OTA (Over-The-Air) Firmware Updates
- [ ] Verschlüsselte Konfiguration (NVS Storage)
- [ ] Sicherheits-Audit
- [ ] Vollständige API-Dokumentation (Swagger/OpenAPI)

**Nutzen:**
- Professionelle Software-Qualität
- Einfache Updates ohne USB
- Sichere Konfiguration

**Aufwand:** L (groß, ca. 4-6 Wochen)

**Risiken:**
- OTA benötigt WiFi-Client-Mode (konfliktiert mit Promiscuous?)
- Sicherheits-Audit aufwändig

---

### v1.1.0 – Advanced UI

**Features:**
- [ ] Button-basiertes Menü-System auf Display
- [ ] Mehrere Screens (Status, History, Settings, Diagnostics)
- [ ] Settings per Button konfigurieren (ohne Web-UI)
- [ ] Heatmap-Visualisierung auf Display
- [ ] Akustisches Feedback (optional, falls Buzzer nachgerüstet)

**Nutzen:**
- Standalone-Nutzung ohne Computer
- Bessere User Experience
- Mehr Informationen auf dem Display

**Aufwand:** M (mittel, ca. 2-3 Wochen)

**Risiken:**
- Button-Menü auf kleinem Display komplex
- Screen-Wechsel kann flackern

---

### v1.2.0 – Plugin-System

**Features:**
- [ ] Plugin-API für Custom Detectors
- [ ] Lua/MicroPython Scripting Support (optional)
- [ ] Hot-Reload für Plugins
- [ ] Community-Plugin Repository
- [ ] Plugin-Manager in Web-UI

**Nutzen:**
- Erweiterbarkeit ohne Core-Code-Änderungen
- Community kann eigene Detektoren entwickeln
- Flexibilität für Sonderfälle

**Aufwand:** L (groß, ca. 4-5 Wochen)

**Risiken:**
- Scripting auf ESP32 sehr ressourcen-intensiv
- Plugin-API-Design komplex
- Sicherheitsrisiken durch Third-Party Code

---

### v1.3.0 – GPS Integration

**Features:**
- [ ] GPS-Modul Support (optional, externe Hardware)
- [ ] Geo-Tagging aller Detections
- [ ] GPX Export für Tracking-Apps
- [ ] Automatische KML-Generierung mit Standorten
- [ ] Offline Maps auf SD-Karte

**Nutzen:**
- Präzise Standort-Daten
- Wardriving-Funktionalität
- Bessere Datenanalyse

**Aufwand:** M (mittel, ca. 2 Wochen)

**Risiken:**
- GPS-Modul benötigt zusätzliche Pins
- Outdoor-Nutzung für GPS-Fix nötig

**Abhängigkeiten:**
- TinyGPS++ Library
- Externe GPS-Hardware

---

## 🔮 Zukünftige Ideen (v2.0+)

**Experimentelle Features (noch nicht priorisiert):**

### Multi-Protocol Support
- [ ] Zigbee Sniffing (falls Hardware-Support)
- [ ] LoRa Detection
- [ ] 5 GHz WiFi Support (ESP32-S3 kann das nicht nativ)

### AI/ML Features
- [ ] On-Device ML Classification
- [ ] Anomaly Detection
- [ ] Behavior Pattern Analysis

### Mesh Networking
- [ ] ESP-NOW Multi-Device-Netzwerk
- [ ] Zentrale Aggregation von mehreren Sensoren
- [ ] Collaborative Detection

### Cloud Integration
- [ ] MQTT Cloud Reporting (optional)
- [ ] Threat Intelligence Feeds
- [ ] Global Detection-Map

---

## 📊 Feature-Priorisierung

**High Priority (v0.1 - v0.3):**
1. ✅ Hardware Diagnostics
2. ✅ Display UI
3. Persistent Logging
4. Web-UI Integration
5. Filter/Whitelist

**Medium Priority (v0.4 - v1.0):**
1. Performance Optimierung
2. OTA Updates
3. Pattern Expansion
4. Button-Menü

**Low Priority (v1.1+):**
1. Plugin-System
2. GPS Integration
3. Advanced ML Features

---

## 🤝 Community Contributions

**Wir freuen uns über Beiträge in folgenden Bereichen:**

### Code
- Bug Fixes
- Performance Improvements
- Neue Detection Patterns

### Dokumentation
- Übersetzungen (Englisch, weitere Sprachen)
- Tutorials und How-Tos
- Video-Guides

### Hardware
- Tests auf verschiedenen Board-Revisionen
- GPS-Modul Integration
- Alternative Display-Größen

### Data
- MAC-Prefix Database erweitern
- SSID-Pattern Sammlung
- Raven Service UUID Forschung

---

## 📅 Geschätzte Timeline

```
2026 Q1: v0.1.0 ✅ (Aktuell)
2026 Q1: v0.2.0 (Logging)
2026 Q2: v0.3.0 (Web-UI)
2026 Q2: v0.4.0 (Performance)
2026 Q3: v0.5.0 (Patterns)
2026 Q3: v1.0.0 (Production)
2026 Q4+: v1.x Features
```

**Hinweis:** Timeline ist abhängig von:
- Community-Beiträgen
- Bug-Reports und Prioritätsänderungen
- Verfügbarer Entwicklungszeit

---

## 🐛 Known Issues / Backlog

**Aktuelle bekannte Probleme:**
- [ ] Display flackert bei schnellem UI-Update (Rate-Limiting prüfen)
- [ ] BLE Scan blockiert WiFi kurzzeitig (NimBLE Task Priority?)
- [ ] Memory-Leak bei Langzeit-Betrieb (24h+) zu testen
- [ ] SD-Karten-Support noch nicht implementiert
- [ ] RTC für Timestamps fehlt (NTP-Integration geplant)

---

## 📝 Changelog-Konvention

```markdown
## [Version] - YYYY-MM-DD

### Added
- Neue Features

### Changed
- Änderungen an bestehenden Features

### Fixed
- Bug Fixes

### Removed
- Entfernte Features

### Security
- Sicherheits-relevante Änderungen
```

---

## 🔗 Weiterführende Links

- **GitHub Repository:** [philibertschlutzki/flock-you](https://github.com/philibertschlutzki/flock-you)
- **Issues/Bug Reports:** [GitHub Issues](https://github.com/philibertschlutzki/flock-you/issues)
- **Discussions:** [GitHub Discussions](https://github.com/philibertschlutzki/flock-you/discussions)

---

**Diese Roadmap ist ein lebendiges Dokument und wird regelmäßig aktualisiert basierend auf Community-Feedback und Entwicklungsprioritäten.**

**Letzte Aktualisierung:** 2026-01-04
