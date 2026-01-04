/**
 * @file detection_patterns.h
 * @brief Flock Safety und Raven Erkennungsmuster
 * 
 * Diese Datei enthält alle bekannten Erkennungsmuster für:
 * - WiFi SSID-Muster
 * - MAC-Adressen-Präfixe
 * - BLE Device-Name-Muster
 * - Raven Gunshot Detector Service UUIDs
 * 
 * Muster wurden aus echten Flock Safety Geräte-Datenbanken extrahiert.
 */

#pragma once

// ============================================================================
// WIFI SSID PATTERNS
// ============================================================================

/**
 * @brief WiFi SSID-Muster die auf Flock Safety Geräte hinweisen
 * 
 * Diese SSIDs wurden in realen Flock Safety Deployments beobachtet.
 */
static const char* wifi_ssid_patterns[] = {
    "flock",
    "Flock",
    "FLOCK",
    "FS Ext Battery",
    "Penguin",
    "Pigvision"
};

// ============================================================================
// MAC ADDRESS PREFIXES
// ============================================================================

/**
 * @brief MAC-Adressen-Präfixe (OUI) von bekannten Flock Safety Geräten
 * 
 * Diese Präfixe identifizieren Hersteller und spezifische Geräteserien.
 */
static const char* mac_prefixes[] = {
    "58:8e:81", "cc:cc:cc", "ec:1b:bd", "90:35:ea", "04:0d:84",
    "f0:82:c0", "1c:34:f1", "38:5b:44", "94:34:69", "b4:e3:f9",
    "70:c9:4e", "3c:91:80", "d8:f3:bc", "80:30:49", "14:5a:fc",
    "74:4c:a1", "08:3a:88", "9c:2f:9d", "94:08:53", "e4:aa:ea"
};

// ============================================================================
// BLE DEVICE NAME PATTERNS
// ============================================================================

/**
 * @brief BLE Gerätenamen-Muster die auf Flock Safety Geräte hinweisen
 */
static const char* device_name_patterns[] = {
    "FS Ext Battery",
    "Penguin",
    "Flock",
    "Pigvision"
};

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
 * @brief Array aller bekannten Raven Service UUIDs
 */
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
