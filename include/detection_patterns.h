/**
 * @file detection_patterns.h
 * @brief Erkennungsmuster (WiFi/BLE) inkl. Wildcards und Filter-Modus (LEGACY/ALLOWLIST).
 *
 * In dieser Datei werden alle Pattern-Listen gepflegt, die eine Erkennung auslösen.
 * Die Arrays werden direkt von den WiFi-/BLE-Modulen verwendet.
 *
 * -----------------------------------------------------------------------------
 * Kurzüberblick: Was wird hier konfiguriert?
 * -----------------------------------------------------------------------------
 * 1) WiFi SSID Patterns        -> wifi_ssid_patterns[]
 * 2) MAC/OUI Patterns          -> mac_prefixes[]
 * 3) BLE Device Name Patterns  -> device_name_patterns[]
 * 4) Raven Service UUIDs       -> raven_service_uuids[]
 *
 * -----------------------------------------------------------------------------
 * Matching-Regeln (für alle Listen)
 * -----------------------------------------------------------------------------
 * - Wildcard: `*` matcht beliebige Zeichenfolge (auch leer)
 * - Ohne `*` wird ein Pattern als exakter Match interpretiert
 * - Alle Vergleiche sind case-insensitiv (Groß-/Kleinschreibung egal)
 * - Leere Strings "" werden vom Code ignoriert (zum Deaktivieren einzelner Einträge)
 *
 * Praxis-Tipp:
 * - Substring-Suche: Pattern vorne und hinten mit `*` umschließen (z.B. `*flock*`).
 *
 * -----------------------------------------------------------------------------
 * Filter-Modus
 * -----------------------------------------------------------------------------
 * - FLOCKYOU_FILTER_MODE_LEGACY   : vordefinierte Heuristik-Listen (Standard-Patterns)
 * - FLOCKYOU_FILTER_MODE_ALLOWLIST: nur die Allowlist-Arrays in diesem File
 *
 * Der Modus wird per Build-Flag gesetzt (PlatformIO), z.B.:
 * - `-DFLOCKYOU_FILTER_MODE=FLOCKYOU_FILTER_MODE_ALLOWLIST`
 *
 * Hinweis zur aktuellen Default-Konfiguration in diesem Repo:
 * - Wenn kein Build-Flag gesetzt ist, ist der Modus hier per #define auf ALLOWLIST
 *   voreingestellt.
 * - Die Allowlist-Arrays sind aktuell mit `"*"` befüllt (catch-all). Für einen
 *   strikten Allowlist-Test diese Einträge bitte durch spezifische Patterns ersetzen.
 *
 * -----------------------------------------------------------------------------
 * Optionale Pattern-Validierung (Boot-Warnungen)
 * -----------------------------------------------------------------------------
 * Manche Builds prüfen Patterns beim Boot auf "zu allgemeine" oder "zu kurze"
 * Wildcards (z.B. match-all), und geben Warnungen aus.
 * In diesem File wird die Validierung standardmäßig deaktiviert, um Warnungen
 * bei kurzen Test-Wildcards zu vermeiden.
 */

#pragma once

// Pattern-Validierung (Boot-Warnungen) deaktivieren.
// Falls ein Build/Projekt ENABLE_WILDCARD_VALIDATION definiert, wird es hier bewusst
// wieder entfernt, um Warnungen (z.B. bei kurzen Test-Wildcards) zu unterdrücken.
#undef ENABLE_WILDCARD_VALIDATION

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
 * Wird FLOCKYOU_FILTER_MODE nicht per Build-Flag gesetzt, gilt der Default unten.
 */
#ifndef FLOCKYOU_FILTER_MODE
#define FLOCKYOU_FILTER_MODE FLOCKYOU_FILTER_MODE_ALLOWLIST
#endif

// ============================================================================
// WIFI SSID PATTERNS
// ============================================================================

/**
 * @brief WiFi-SSID Pattern-Liste.
 *
 * Matcht gegen SSIDs aus Probe Requests/Beacons.
 * Wildcards `*` sind erlaubt (siehe Header-Kommentar "Matching-Regeln").
 */
#if FLOCKYOU_FILTER_MODE == FLOCKYOU_FILTER_MODE_ALLOWLIST
static const char* wifi_ssid_patterns[] = {
    "*" // Catch-all wildcard to match everything
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
 * @brief MAC Patterns (typisch: OUI/Herstellerpräfix) für WiFi und BLE.
 *
 * Format:
 * - MAC-Notation mit Doppelpunkt: "aa:bb:cc:dd:ee:ff"
 * - Wildcards: z.B. "aa:bb:cc:*", "58:8e:*", "*:dd:ee:ff"
 */
#if FLOCKYOU_FILTER_MODE == FLOCKYOU_FILTER_MODE_ALLOWLIST
static const char* mac_prefixes[] = {
    "*" // Catch-all wildcard to match everything
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
 * @brief BLE Gerätenamen-Pattern-Liste.
 *
 * Matcht gegen den beworbenen BLE-Namen (Advertisement).
 * Wildcards `*` sind erlaubt (siehe Header-Kommentar "Matching-Regeln").
 */
#if FLOCKYOU_FILTER_MODE == FLOCKYOU_FILTER_MODE_ALLOWLIST
static const char* device_name_patterns[] = {
    "*" // Catch-all wildcard to match everything
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
 * @brief Liste aller bekannten Raven Service UUIDs (inkl. Wildcard-Support).
 *
 * Format:
 * - 128-bit UUID-Strings
 * - Wildcards: z.B. "00003100-*", "*-00805f9b34fb"
 */
#if FLOCKYOU_FILTER_MODE == FLOCKYOU_FILTER_MODE_ALLOWLIST
static const char* raven_service_uuids[] = {
    "*" // Catch-all wildcard to match everything
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
