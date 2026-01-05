/**
 * @file pattern_startup_check.cpp
 * @brief Startup-Validierung aller Erkennungsmuster
 * 
 * Validiert beim Boot alle Pattern-Arrays und loggt ungültige Einträge.
 */

#include "pattern_validator.h"
#include "detection_patterns.h"
#include <Arduino.h>

/**
 * @brief Validiert alle WiFi SSID Patterns beim Startup
 */
static void validate_wifi_ssid_patterns()
{
    Serial.println("[PATTERN_CHECK] Validating WiFi SSID patterns...");
    int valid_count = 0;
    int invalid_count = 0;
    
    for (int i = 0; i < (int)(sizeof(wifi_ssid_patterns)/sizeof(wifi_ssid_patterns[0])); i++) {
        const char* p = wifi_ssid_patterns[i];
        if (!p || !*p) continue; // Leere Patterns überspringen
        
        if (validate_pattern(p, PATTERN_TYPE_SSID)) {
            valid_count++;
        } else {
            invalid_count++;
            Serial.printf("[PATTERN_CHECK] ERROR: Invalid WiFi SSID pattern #%d: '%s'\n", i, p);
            Serial.printf("                Reason: %s\n", get_validation_error(p, PATTERN_TYPE_SSID));
        }
    }
    
    Serial.printf("[PATTERN_CHECK] WiFi SSID: %d valid, %d invalid\n", valid_count, invalid_count);
    
#ifdef STRICT_PATTERN_VALIDATION
    if (invalid_count > 0) {
        Serial.println("[PATTERN_CHECK] FATAL: Invalid patterns found with STRICT_PATTERN_VALIDATION enabled!");
        Serial.println("[PATTERN_CHECK] Please fix the patterns in include/detection_patterns.h");
        while (true) {
            delay(1000); // Boot abort
        }
    }
#endif
}

/**
 * @brief Validiert alle MAC-Adress Patterns beim Startup
 */
static void validate_mac_patterns()
{
    Serial.println("[PATTERN_CHECK] Validating MAC address patterns...");
    int valid_count = 0;
    int invalid_count = 0;
    
    for (int i = 0; i < (int)(sizeof(mac_prefixes)/sizeof(mac_prefixes[0])); i++) {
        const char* p = mac_prefixes[i];
        if (!p || !*p) continue; // Leere Patterns überspringen
        
        if (validate_pattern(p, PATTERN_TYPE_MAC)) {
            valid_count++;
        } else {
            invalid_count++;
            Serial.printf("[PATTERN_CHECK] ERROR: Invalid MAC pattern #%d: '%s'\n", i, p);
            Serial.printf("                Reason: %s\n", get_validation_error(p, PATTERN_TYPE_MAC));
        }
    }
    
    Serial.printf("[PATTERN_CHECK] MAC patterns: %d valid, %d invalid\n", valid_count, invalid_count);
    
#ifdef STRICT_PATTERN_VALIDATION
    if (invalid_count > 0) {
        Serial.println("[PATTERN_CHECK] FATAL: Invalid patterns found with STRICT_PATTERN_VALIDATION enabled!");
        Serial.println("[PATTERN_CHECK] Please fix the patterns in include/detection_patterns.h");
        while (true) {
            delay(1000); // Boot abort
        }
    }
#endif
}

/**
 * @brief Validiert alle BLE Device Name Patterns beim Startup
 */
static void validate_ble_name_patterns()
{
    Serial.println("[PATTERN_CHECK] Validating BLE device name patterns...");
    int valid_count = 0;
    int invalid_count = 0;
    
    for (int i = 0; i < (int)(sizeof(device_name_patterns)/sizeof(device_name_patterns[0])); i++) {
        const char* p = device_name_patterns[i];
        if (!p || !*p) continue; // Leere Patterns überspringen
        
        if (validate_pattern(p, PATTERN_TYPE_SSID)) {
            valid_count++;
        } else {
            invalid_count++;
            Serial.printf("[PATTERN_CHECK] ERROR: Invalid BLE name pattern #%d: '%s'\n", i, p);
            Serial.printf("                Reason: %s\n", get_validation_error(p, PATTERN_TYPE_SSID));
        }
    }
    
    Serial.printf("[PATTERN_CHECK] BLE name patterns: %d valid, %d invalid\n", valid_count, invalid_count);
    
#ifdef STRICT_PATTERN_VALIDATION
    if (invalid_count > 0) {
        Serial.println("[PATTERN_CHECK] FATAL: Invalid patterns found with STRICT_PATTERN_VALIDATION enabled!");
        Serial.println("[PATTERN_CHECK] Please fix the patterns in include/detection_patterns.h");
        while (true) {
            delay(1000); // Boot abort
        }
    }
#endif
}

/**
 * @brief Validiert alle Raven Service UUID Patterns beim Startup
 */
static void validate_raven_uuid_patterns()
{
    Serial.println("[PATTERN_CHECK] Validating Raven Service UUID patterns...");
    int valid_count = 0;
    int invalid_count = 0;
    
    for (int i = 0; i < (int)(sizeof(raven_service_uuids)/sizeof(raven_service_uuids[0])); i++) {
        const char* p = raven_service_uuids[i];
        if (!p || !*p) continue; // Leere Patterns überspringen
        
        if (validate_pattern(p, PATTERN_TYPE_UUID)) {
            valid_count++;
        } else {
            invalid_count++;
            Serial.printf("[PATTERN_CHECK] ERROR: Invalid Raven UUID pattern #%d: '%s'\n", i, p);
            Serial.printf("                Reason: %s\n", get_validation_error(p, PATTERN_TYPE_UUID));
        }
    }
    
    Serial.printf("[PATTERN_CHECK] Raven UUID patterns: %d valid, %d invalid\n", valid_count, invalid_count);
    
#ifdef STRICT_PATTERN_VALIDATION
    if (invalid_count > 0) {
        Serial.println("[PATTERN_CHECK] FATAL: Invalid patterns found with STRICT_PATTERN_VALIDATION enabled!");
        Serial.println("[PATTERN_CHECK] Please fix the patterns in include/detection_patterns.h");
        while (true) {
            delay(1000); // Boot abort
        }
    }
#endif
}

/**
 * @brief Validiert alle Erkennungsmuster beim Boot
 * 
 * Sollte in main.cpp vor dem Start der Scanner aufgerufen werden.
 * Loggt ungültige Patterns und kann optional Boot abbrechen (STRICT_PATTERN_VALIDATION).
 */
void validate_all_patterns()
{
    Serial.println("\n========================================");
    Serial.println("[PATTERN_CHECK] Starting pattern validation...");
    Serial.println("========================================");
    
    validate_wifi_ssid_patterns();
    validate_mac_patterns();
    validate_ble_name_patterns();
    validate_raven_uuid_patterns();
    
    Serial.println("========================================");
    Serial.println("[PATTERN_CHECK] Pattern validation complete");
    Serial.println("========================================\n");
}
