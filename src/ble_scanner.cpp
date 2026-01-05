/**
 * @file ble_scanner.cpp
 * @brief BLE Scanner Implementierung
 */

#include "ble_scanner.h"
#include "config.h"
#include "detection_patterns.h"
#include "output.h"
#include "state.h"
#include "wildcard_match.h"
#include "pattern_validator.h"
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <ArduinoJson.h>
#include <string.h>

// Globale Variablen für BLE Scanner
static NimBLEScan* pBLEScan = nullptr;
static unsigned long last_ble_scan = 0;

/**
 * @brief Prüft ob BLE Service UUID zu Raven Gunshot Detector gehört
 * 
 * @param device BLE Advertised Device
 * @param detected_service_out Optional: Output-Buffer für erkannte UUID (min 41 Bytes)
 * @return true wenn Raven Service UUID gefunden, sonst false
 */
static bool check_raven_service_uuid(NimBLEAdvertisedDevice* device, char* detected_service_out = nullptr)
{
    if (!device) return false;
    if (!device->haveServiceUUID()) return false;

    int serviceCount = device->getServiceUUIDCount();
    if (serviceCount == 0) return false;

    for (int i = 0; i < serviceCount; i++) {
        NimBLEUUID serviceUUID = device->getServiceUUID(i);
        std::string uuidStr = serviceUUID.toString();

        for (int j = 0; j < (int)(sizeof(raven_service_uuids)/sizeof(raven_service_uuids[0])); j++) {
            const char* p = raven_service_uuids[j];
            if (!p || !*p) continue; // Allowlist kann leer sein
            
            // Validiere Pattern (nur einmalig warnen bei ungültigen Patterns)
#ifdef ENABLE_WILDCARD_VALIDATION
            if (!validate_pattern(p, PATTERN_TYPE_UUID)) {
                static bool warned[32] = {false};
                if (j < 32 && !warned[j]) {
                    Serial.printf("[WILDCARD] Invalid UUID pattern #%d: '%s' - %s\n", 
                        j, p, get_validation_error(p, PATTERN_TYPE_UUID));
                    warned[j] = true;
                }
                continue;
            }
#endif
            
            if (wildcard_match_uuid(p, uuidStr.c_str())) {
                if (detected_service_out != nullptr) {
                    strncpy(detected_service_out, uuidStr.c_str(), 40);
                }
                return true;
            }
        }
    }

    return false;
}

/**
 * @brief Gibt lesbare Beschreibung für Raven Service UUID
 * 
 * @param uuid Service UUID String
 * @return Lesbare Service-Beschreibung
 */
static const char* get_raven_service_description(const char* uuid)
{
    if (!uuid) return "Unknown Service";

    if (strcasecmp(uuid, RAVEN_DEVICE_INFO_SERVICE) == 0) return "Device Information";
    if (strcasecmp(uuid, RAVEN_GPS_SERVICE) == 0) return "GPS Location";
    if (strcasecmp(uuid, RAVEN_POWER_SERVICE) == 0) return "Power Management";
    if (strcasecmp(uuid, RAVEN_NETWORK_SERVICE) == 0) return "Network Status";
    if (strcasecmp(uuid, RAVEN_UPLOAD_SERVICE) == 0) return "Upload Statistics";
    if (strcasecmp(uuid, RAVEN_ERROR_SERVICE) == 0) return "Error/Failure";
    if (strcasecmp(uuid, RAVEN_OLD_HEALTH_SERVICE) == 0) return "Health (Legacy)";
    if (strcasecmp(uuid, RAVEN_OLD_LOCATION_SERVICE) == 0) return "Location (Legacy)";

    return "Unknown Raven Service";
}

/**
 * @brief Schätzt Raven Firmware-Version anhand vorhandener Service UUIDs
 * 
 * @param device BLE Advertised Device
 * @return Geschätzte Firmware-Version als String
 */
static const char* estimate_raven_firmware_version(NimBLEAdvertisedDevice* device)
{
    if (!device || !device->haveServiceUUID()) return "Unknown";

    bool has_new_gps = false;
    bool has_old_location = false;
    bool has_power_service = false;

    int serviceCount = device->getServiceUUIDCount();
    for (int i = 0; i < serviceCount; i++) {
        NimBLEUUID serviceUUID = device->getServiceUUID(i);
        std::string uuidStr = serviceUUID.toString();

        if (strcasecmp(uuidStr.c_str(), RAVEN_GPS_SERVICE) == 0) has_new_gps = true;
        if (strcasecmp(uuidStr.c_str(), RAVEN_OLD_LOCATION_SERVICE) == 0) has_old_location = true;
        if (strcasecmp(uuidStr.c_str(), RAVEN_POWER_SERVICE) == 0) has_power_service = true;
    }

    if (has_old_location && !has_new_gps) return "1.1.x (Legacy)";
    if (has_new_gps && !has_power_service) return "1.2.x";
    if (has_new_gps && has_power_service) return "1.3.x (Latest)";

    return "Unknown Version";
}

/**
 * @brief Prüft ob MAC-Adresse einem bekannten Flock Safety Präfix entspricht
 * @param mac MAC-Adresse (6 Bytes)
 * @return true wenn MAC-Präfix übereinstimmt, sonst false
 */
static bool check_mac_prefix(const uint8_t* mac)
{
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    for (int i = 0; i < (int)(sizeof(mac_prefixes)/sizeof(mac_prefixes[0])); i++) {
        const char* p = mac_prefixes[i];
        if (!p || !*p) continue; // Allowlist kann leer sein
        
        // Validiere Pattern (nur einmalig warnen bei ungültigen Patterns)
#ifdef ENABLE_WILDCARD_VALIDATION
        if (!validate_pattern(p, PATTERN_TYPE_MAC)) {
            static bool warned[64] = {false};
            if (i < 64 && !warned[i]) {
                Serial.printf("[WILDCARD] Invalid MAC pattern #%d: '%s' - %s\n", 
                    i, p, get_validation_error(p, PATTERN_TYPE_MAC));
                warned[i] = true;
            }
            continue;
        }
#endif
        
        if (wildcard_match_mac(p, mac_str)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief BLE Advertisement Callback-Handler
 * 
 * Wird für jedes empfangene BLE Advertisement aufgerufen.
 * Prüft auf Flock Safety und Raven Geräte.
 */
class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override {

        NimBLEAddress addr = advertisedDevice->getAddress();
        std::string addrStr = addr.toString();
        uint8_t mac[6];
        sscanf(addrStr.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

        int rssi = advertisedDevice->getRSSI();
        std::string name = "";
        if (advertisedDevice->haveName()) {
            name = advertisedDevice->getName();
        }

        // Prüfe MAC-Präfix
        if (check_mac_prefix(mac)) {
            output_ble_detection_json(addrStr.c_str(), name.c_str(), rssi, "mac_prefix");
            if (!triggered) {
                output_trigger_detection();
                triggered = true;
            }
            last_detection_time = millis();
            return;
        }

        // Prüfe Device-Name
        if (!name.empty() && ble_check_device_name_pattern(name.c_str())) {
            output_ble_detection_json(addrStr.c_str(), name.c_str(), rssi, "device_name");
            if (!triggered) {
                output_trigger_detection();
                triggered = true;
            }
            last_detection_time = millis();
            return;
        }

        // Prüfe Raven Service UUIDs
        char detected_service_uuid[41] = {0};
        if (check_raven_service_uuid(advertisedDevice, detected_service_uuid)) {
            const char* fw_version = estimate_raven_firmware_version(advertisedDevice);
            const char* service_desc = get_raven_service_description(detected_service_uuid);

            // Zähle erkannte Raven Services
            int raven_service_count = 0;
            if (advertisedDevice->haveServiceUUID()) {
                int serviceCount = advertisedDevice->getServiceUUIDCount();
                for (int i = 0; i < serviceCount; i++) {
                    NimBLEUUID serviceUUID = advertisedDevice->getServiceUUID(i);
                    std::string uuidStr = serviceUUID.toString();
                    for (int j = 0; j < (int)(sizeof(raven_service_uuids)/sizeof(raven_service_uuids[0])); j++) {
                        const char* p = raven_service_uuids[j];
                        if (!p || !*p) continue;
                        if (strcasecmp(uuidStr.c_str(), p) == 0) {
                            raven_service_count++;
                            break;
                        }
                    }
                }
            }

            output_raven_detection_json(addrStr.c_str(), rssi, detected_service_uuid, raven_service_count);

            if (!triggered) {
                output_trigger_detection();
                triggered = true;
            }
            last_detection_time = millis();
            return;
        }
    }
};

void ble_scanner_init()
{
    Serial.println("Initializing BLE scanner...");
    NimBLEDevice::init("");
    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    Serial.println("BLE scanner initialized");
}

void ble_scanner_tick()
{
    if (!pBLEScan) return;

    if (millis() - last_ble_scan >= BLE_SCAN_INTERVAL && !pBLEScan->isScanning()) {
        Serial.println("[BLE] scan...");
        pBLEScan->start(BLE_SCAN_DURATION, false);
        last_ble_scan = millis();
    }

    if (!pBLEScan->isScanning() && millis() - last_ble_scan > BLE_SCAN_DURATION * 1000) {
        pBLEScan->clearResults();
    }
}

bool ble_scanner_is_scanning()
{
    if (!pBLEScan) return false;
    return pBLEScan->isScanning();
}

bool ble_check_device_name_pattern(const char* name)
{
    if (!name) return false;

    for (int i = 0; i < (int)(sizeof(device_name_patterns)/sizeof(device_name_patterns[0])); i++) {
        const char* p = device_name_patterns[i];
        if (!p || !*p) continue; // Allowlist kann leer sein
        
        // Validiere Pattern (nur einmalig warnen bei ungültigen Patterns)
#ifdef ENABLE_WILDCARD_VALIDATION
        if (!validate_pattern(p, PATTERN_TYPE_SSID)) {
            static bool warned[32] = {false};
            if (i < 32 && !warned[i]) {
                Serial.printf("[WILDCARD] Invalid BLE name pattern #%d: '%s' - %s\n", 
                    i, p, get_validation_error(p, PATTERN_TYPE_SSID));
                warned[i] = true;
            }
            continue;
        }
#endif
        
        if (wildcard_match_ci(p, name)) {
            return true;
        }
    }
    return false;
}
