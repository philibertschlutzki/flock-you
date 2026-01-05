/**
 * @file detection_patterns.h
 * @brief Erkennungsmuster (WiFi/BLE) und optionaler Allowlist-Modus mit Wildcard-Support.
 *
 * Diese Datei definiert alle Musterlisten, die die Erkennung auslösen.
 * Die Listen werden von `wifi_sniffer.cpp`, `ble_scanner.cpp` und `output.cpp`
 * direkt verwendet. [file:2][file:3][file:4]
 *
 * ## Matching-Semantik (mit Wildcard-Support)
 * - WiFi SSID: **Wildcard-Matching** (`*` = beliebige Zeichenfolge) [file:2]
 * - BLE Name: **Wildcard-Matching** (`*` = beliebige Zeichenfolge) [file:3]
 * - MAC Prefix/Pattern: Wildcard-Matching mit `*` (z.B. `aa:bb:*`, `*:dd:ee:ff`) [file:2][file:3]
 * - Raven Service UUID: Wildcard-Matching mit `*` (z.B. `00003100-*`) [file:3]
 * - Alle Vergleiche sind **case-insensitiv**
 *
 * ## Wildcard-Syntax
 * - `*` = beliebige Zeichenfolge (inkl. leer)
 * - Patterns ohne `*` werden als exakte Matches interpretiert (Backward-Compatible)
 * 
 * ### Beispiele für gültige Patterns:
 * - SSID: `"RAVEN-*"`, `"*-Guest"`, `"Flock*Camera"`, `"MyNetwork"`
 * - MAC: `"aa:bb:cc:*"`, `"58:8e:*"`, `"*:dd:ee:ff"`, `"aa:bb:cc:dd:ee:ff"`
 * - UUID: `"00003100-*"`, `"*-00805f9b34fb"`, `"00003100-*-00805f9b34fb"`
 *
 * ### Ungültige Patterns (werden beim Boot gewarnt):
 * - Reine Wildcards: `"*"`, `"**"` (= match-all Prevention)
 * - Zu kurze SSID/Name: `"a*"`, `"ab*"` (min. 3 Literal-Zeichen erforderlich)
 * - Zu kurze MAC: `"*"` ohne vollständiges Byte (min. 1 Byte `aa:` erforderlich)
 * - Zu kurze UUID: `"00*"` (min. 8 zusammenhängende Hex-Zeichen erforderlich)
 *
 * ## Filter-Modus
 * Standardmäßig sind die unten stehenden "Legacy"-Listen aktiv.
 * Für kontrollierte Tests (z.B. Labor/Own Devices) kann auf "ALLOWLIST" umgestellt werden.
 * Dann werden ausschließlich die Allowlist-Listen ausgewertet.
 *
 * Aktivierung (PlatformIO / build flags):
 * - `-DFLOCKYOU_FILTER_MODE=FLOCKYOU_FILTER_MODE_ALLOWLIST`
 *
 * WICHTIG: Allowlist-Listen sind per Default so initialisiert, dass sie **nichts matchen**.
 * Leere/"disabled"-Einträge werden vom Code übersprungen.
 */

#pragma once

// ============================================================================
// FILTER MODE SWITCH
// ============================================================================

/**
 * @brief Filter-Modus: Standard (Legacy-Patterns)
 */
#define FLOCKYOU_FILTER_MODE_LEGACY 0

/**
 * @brief Filter-Modus: Strikte Allowlist (nur eigene, explizit eingetragene Geräte)
 */
#define FLOCKYOU_FILTER_MODE_ALLOWLIST 1

/**
 * @brief Aktiver Filter-Modus.
 *
 * Default ist LEGACY.
 */
#ifndef FLOCKYOU_FILTER_MODE
#define FLOCKYOU_FILTER_MODE FLOCKYOU_FILTER_MODE_LEGACY
#endif

// ============================================================================
// WIFI SSID PATTERNS
// ============================================================================

/**
 * @brief WiFi-SSID Musterliste mit Wildcard-Support.
 *
 * Verwendung:
 * - `wifi_sniffer.cpp::wifi_check_ssid_pattern()` prüft `wildcard_match_ci(pattern, ssid)` [file:2]
 * - `output.cpp::output_wifi_detection_json()` schreibt `matched_ssid_pattern` in JSON [file:4]
 *
 * Format:
 * - Einträge können Wildcards enthalten: `*` = beliebige Zeichenfolge
 * - Patterns ohne `*` werden als exakte Matches interpretiert
 * - Mindestens 3 Literal-Zeichen erforderlich (z.B. `"RAVEN-*"` OK, `"a*"` NICHT OK)
 * - Leere Strings werden ignoriert (damit Allowlist leer bleiben kann)
 * 
 * Beispiele:
 * - `"RAVEN-*"` matcht `"RAVEN-123"`, `"RAVEN-XYZ"`
 * - `"*-Guest"` matcht `"MyNet-Guest"`, `"Office-Guest"`
 * - `"Flock*Camera"` matcht `"FlockStreetCamera"`, `"Flock_Camera"`
 * - `"Flock"` matcht nur exakt `"Flock"` (kein Wildcard)
 */
#if FLOCKYOU_FILTER_MODE == FLOCKYOU_FILTER_MODE_ALLOWLIST
static const char* wifi_ssid_patterns[] = {
    "" // Allowlist leer = keine SSID-Matches. Trage hier Substrings ein.
    // Beispiel: "MyTestSSID"
};
#else
static const char* wifi_ssid_patterns[] = {
    // --- Patterns mit Wildcard-Support ---
    // Hinweis: Um altes Substring-Verhalten zu erhalten, wurde "flock" → "*flock*" umgewandelt
    "*flock*",        // Matcht überall "flock" (backward-compatible)
    "*Flock*",        // Matcht überall "Flock" (backward-compatible)
    "*FLOCK*",        // Matcht überall "FLOCK" (backward-compatible)
    "*FS Ext Battery*",  // Matcht überall "FS Ext Battery"
    "*Penguin*",      // Matcht überall "Penguin"
    "*Pigvision*"     // Matcht überall "Pigvision"
};
#endif

// ============================================================================
// MAC ADDRESS PREFIXES (OUI)
// ============================================================================

/**
 * @brief MAC-Präfixe/Patterns (OUI) mit Wildcard-Support.
 *
 * Verwendung:
 * - WiFi: `wifi_sniffer.cpp::wifi_check_mac_prefix()` vergleicht via `wildcard_match_mac()` [file:2]
 * - BLE:  `ble_scanner.cpp::check_mac_prefix()` vergleicht ebenfalls via Wildcard [file:3]
 * - Output: JSON enthält `matched_mac_pattern` [file:4]
 *
 * Format-Regeln:
 * - Standard: 6 Bytes als Hex mit Doppelpunkten: `"58:8e:81:12:34:56"`
 * - Wildcards erlaubt: `"aa:bb:cc:*"`, `"58:8e:*"`, `"*:dd:ee:ff"`
 * - Mindestens 1 vollständiges Byte erforderlich (z.B. `"aa:*"` OK, `"*"` NICHT OK)
 * - Groß-/Kleinschreibung egal (Vergleich ist case-insensitiv)
 * - Leere Strings werden ignoriert
 * 
 * Beispiele:
 * - `"aa:bb:cc:*"` matcht alle MACs mit Präfix `aa:bb:cc`
 * - `"58:8e:*"` matcht alle MACs von Hersteller mit OUI `58:8e`
 * - `"*:dd:ee:ff"` matcht alle MACs mit Suffix `dd:ee:ff`
 */
#if FLOCKYOU_FILTER_MODE == FLOCKYOU_FILTER_MODE_ALLOWLIST
static const char* mac_prefixes[] = {
    "" // Allowlist leer = keine MAC-Matches. Trage hier OUIs ein, z.B. "aa:bb:cc".
};
#else
static const char* mac_prefixes[] = {
    // --- Patterns mit Wildcard-Support (OUI Präfixe) ---
    // Hinweis: Vollständige MAC-Präfixe (3 Bytes) mit Wildcard für Suffix
    "58:8e:81:*", "cc:cc:cc:*", "ec:1b:bd:*", "90:35:ea:*", "04:0d:84:*",
    "f0:82:c0:*", "1c:34:f1:*", "38:5b:44:*", "94:34:69:*", "b4:e3:f9:*",
    "70:c9:4e:*", "3c:91:80:*", "d8:f3:bc:*", "80:30:49:*", "14:5a:fc:*",
    "74:4c:a1:*", "08:3a:88:*", "9c:2f:9d:*", "94:08:53:*", "e4:aa:ea:*"
};
#endif

// ============================================================================
// BLE DEVICE NAME PATTERNS
// ============================================================================

/**
 * @brief BLE Gerätenamen-Musterliste mit Wildcard-Support.
 *
 * Verwendung:
 * - `ble_scanner.cpp::ble_check_device_name_pattern()` prüft `wildcard_match_ci(pattern, name)` [file:3]
 * - `output.cpp::output_ble_detection_json()` schreibt `matched_name_pattern` [file:4]
 *
 * Format:
 * - Einträge können Wildcards enthalten: `*` = beliebige Zeichenfolge
 * - Patterns ohne `*` werden als exakte Matches interpretiert
 * - Mindestens 3 Literal-Zeichen erforderlich
 * - Leere Strings werden ignoriert
 * 
 * Beispiele siehe WiFi SSID Patterns (gleiche Syntax)
 */
#if FLOCKYOU_FILTER_MODE == FLOCKYOU_FILTER_MODE_ALLOWLIST
static const char* device_name_patterns[] = {
    "" // Allowlist leer = keine Name-Matches. Trage hier Substrings ein.
};
#else
static const char* device_name_patterns[] = {
    // --- Patterns mit Wildcard-Support ---
    // Hinweis: Um altes Substring-Verhalten zu erhalten, wurden Patterns mit * umgeben
    "*FS Ext Battery*",
    "*Penguin*",
    "*Flock*",
    "*Pigvision*"
};
#endif

// ============================================================================
// RAVEN SURVEILLANCE DEVICE UUID PATTERNS
// ============================================================================

/**
 * @brief Raven Gunshot Detector - Device Information Service
 */
#define RAVEN_DEVICE_INFO_SERVICE       "0000180a-0000-1000-8000-00805f9b34fb"

/**
 * @brief Raven Gunshot Detector - GPS Location Service
 */
#define RAVEN_GPS_SERVICE               "00003100-0000-1000-8000-00805f9b34fb"

/**
 * @brief Raven Gunshot Detector - Power Management Service
 */
#define RAVEN_POWER_SERVICE             "00003200-0000-1000-8000-00805f9b34fb"

/**
 * @brief Raven Gunshot Detector - Network Status Service
 */
#define RAVEN_NETWORK_SERVICE           "00003300-0000-1000-8000-00805f9b34fb"

/**
 * @brief Raven Gunshot Detector - Upload Statistics Service
 */
#define RAVEN_UPLOAD_SERVICE            "00003400-0000-1000-8000-00805f9b34fb"

/**
 * @brief Raven Gunshot Detector - Error/Failure Service
 */
#define RAVEN_ERROR_SERVICE             "00003500-0000-1000-8000-00805f9b34fb"

/**
 * @brief Raven Gunshot Detector - Health Service (Legacy)
 */
#define RAVEN_OLD_HEALTH_SERVICE        "00001809-0000-1000-8000-00805f9b34fb"

/**
 * @brief Raven Gunshot Detector - Location Service (Legacy)
 */
#define RAVEN_OLD_LOCATION_SERVICE      "00001819-0000-1000-8000-00805f9b34fb"

/**
 * @brief Liste aller bekannten Raven Service UUIDs mit Wildcard-Support.
 *
 * Verwendung:
 * - `ble_scanner.cpp::check_raven_service_uuid()` vergleicht via `wildcard_match_uuid()` [file:3]
 *
 * Format:
 * - UUID-Strings im üblichen 128-bit Format (lower/upper egal)
 * - Wildcards erlaubt: `"00003100-*"`, `"*-00805f9b34fb"`
 * - Mindestens 8 zusammenhängende Hex-Zeichen erforderlich
 * 
 * Beispiele:
 * - `"00003100-*"` matcht alle UUIDs mit Präfix `00003100`
 * - `"*-00805f9b34fb"` matcht alle UUIDs mit Suffix `00805f9b34fb`
 */
#if FLOCKYOU_FILTER_MODE == FLOCKYOU_FILTER_MODE_ALLOWLIST
static const char* raven_service_uuids[] = {
    "" // Allowlist leer = keine Raven-UUID-Matches. Trage hier UUID-Strings ein.
};
#else
static const char* raven_service_uuids[] = {
    RAVEN_DEVICE_INFO_SERVICE,
    RAVEN_GPS_SERVICE,
    RAVEN_POWER_SERVICE,
    RAVEN_NETWORK_SERVICE,
    RAVEN_UPLOAD_SERVICE,
    RAVEN_ERROR_SERVICE,
    RAVEN_OLD_HEALTH_SERVICE,
    RAVEN_OLD_LOCATION_SERVICE
};
#endif
