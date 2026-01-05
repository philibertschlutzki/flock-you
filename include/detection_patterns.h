/**
 * @file detection_patterns.h
 * @brief Erkennungsmuster (WiFi/BLE) und optionaler Allowlist-Modus.
 *
 * Diese Datei definiert alle Musterlisten, die die Erkennung auslösen.
 * Die Listen werden von `wifi_sniffer.cpp`, `ble_scanner.cpp` und `output.cpp`
 * direkt verwendet. [file:2][file:3][file:4]
 *
 * ## Matching-Semantik (aktueller Code)
 * - WiFi SSID: **case-insensitives Substring-Matching** (`strcasestr`) [file:2]
 * - BLE Name: **case-insensitives Substring-Matching** (`strcasestr`) [file:3]
 * - MAC Prefix (OUI): Vergleich auf die ersten 3 Bytes als String `aa:bb:cc` [file:2][file:3]
 * - Raven Service UUID: **exakter** (case-insensitiver) Vergleich des UUID-Strings [file:3]
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
 * @brief WiFi-SSID Musterliste.
 *
 * Verwendung:
 * - `wifi_sniffer.cpp::wifi_check_ssid_pattern()` prüft `strcasestr(ssid, pattern)` [file:2]
 * - `output.cpp::output_wifi_detection_json()` schreibt `matched_ssid_pattern` in JSON [file:4]
 *
 * Format:
 * - Einträge sind Substrings (keine Wildcards), z.B. "Flock" matcht auch "Flock-123".
 * - Leere Strings werden ignoriert (damit Allowlist leer bleiben kann).
 */
#if FLOCKYOU_FILTER_MODE == FLOCKYOU_FILTER_MODE_ALLOWLIST
static const char* wifi_ssid_patterns[] = {
    "" // Allowlist leer = keine SSID-Matches. Trage hier Substrings ein.
    // Beispiel: "MyTestSSID"
};
#else
static const char* wifi_ssid_patterns[] = {
    // --- Legacy Patterns (aus beobachteten Deployments) ---
    "flock",
    "Flock",
    "FLOCK",
    "FS Ext Battery",
    "Penguin",
    "Pigvision"
};
#endif

// ============================================================================
// MAC ADDRESS PREFIXES (OUI)
// ============================================================================

/**
 * @brief MAC-Präfixe (OUI) im Format `aa:bb:cc`.
 *
 * Verwendung:
 * - WiFi: `wifi_sniffer.cpp::wifi_check_mac_prefix()` vergleicht 8 Zeichen (`aa:bb:cc`) [file:2]
 * - BLE:  `ble_scanner.cpp::check_mac_prefix()` vergleicht ebenfalls nur Präfix [file:3]
 * - Output: JSON enthält `matched_mac_pattern` [file:4]
 *
 * Format-Regeln:
 * - Genau 3 Bytes als Hex, mit Doppelpunkten: `58:8e:81`
 * - Groß-/Kleinschreibung egal (Vergleich ist case-insensitiv).
 * - Leere Strings werden ignoriert.
 */
#if FLOCKYOU_FILTER_MODE == FLOCKYOU_FILTER_MODE_ALLOWLIST
static const char* mac_prefixes[] = {
    "" // Allowlist leer = keine MAC-Matches. Trage hier OUIs ein, z.B. "aa:bb:cc".
};
#else
static const char* mac_prefixes[] = {
    // --- Legacy Patterns (Auszug) ---
    "58:8e:81", "cc:cc:cc", "ec:1b:bd", "90:35:ea", "04:0d:84",
    "f0:82:c0", "1c:34:f1", "38:5b:44", "94:34:69", "b4:e3:f9",
    "70:c9:4e", "3c:91:80", "d8:f3:bc", "80:30:49", "14:5a:fc",
    "74:4c:a1", "08:3a:88", "9c:2f:9d", "94:08:53", "e4:aa:ea"
};
#endif

// ============================================================================
// BLE DEVICE NAME PATTERNS
// ============================================================================

/**
 * @brief BLE Gerätenamen-Musterliste.
 *
 * Verwendung:
 * - `ble_scanner.cpp::ble_check_device_name_pattern()` prüft `strcasestr(name, pattern)` [file:3]
 * - `output.cpp::output_ble_detection_json()` schreibt `matched_name_pattern` [file:4]
 *
 * Format:
 * - Einträge sind Substrings (keine Wildcards).
 * - Leere Strings werden ignoriert.
 */
#if FLOCKYOU_FILTER_MODE == FLOCKYOU_FILTER_MODE_ALLOWLIST
static const char* device_name_patterns[] = {
    "" // Allowlist leer = keine Name-Matches. Trage hier Substrings ein.
};
#else
static const char* device_name_patterns[] = {
    "FS Ext Battery",
    "Penguin",
    "Flock",
    "Pigvision"
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
 * @brief Liste aller bekannten Raven Service UUIDs.
 *
 * Verwendung:
 * - `ble_scanner.cpp::check_raven_service_uuid()` vergleicht `uuid.toString()` exaktes Match [file:3]
 *
 * Format:
 * - UUID-Strings im üblichen 128-bit Format (lower/upper egal).
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
