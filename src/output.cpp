/**
 * @file output.cpp
 * @brief Ausgabe-Funktionen Implementierung
 */

#include "output.h"
#include "config.h"
#include "detection_patterns.h"
#include "state.h"
#include "wifi_sniffer.h"
#include "wildcard_match.h"
#include "pattern_validator.h"
#include <ArduinoJson.h>

#if FLOCKYOU_HAS_DISPLAY
#include "ui_display.h"
#endif

#if !FLOCKYOU_NO_BUZZER
/**
 * @brief Spielt einen Piepton ab
 * @param frequency Frequenz in Hz
 * @param duration_ms Dauer in Millisekunden
 */
static void beep(int frequency, int duration_ms)
{
    tone(BUZZER_PIN, frequency, duration_ms);
    delay(duration_ms + 50);
}
#endif

void output_wifi_detection_json(const char* ssid, const uint8_t* mac, int rssi, const char* detection_type)
{
    DynamicJsonDocument doc(2048);

    doc["timestamp"] = millis();
    doc["detection_time"] = String(millis() / 1000.0, 3) + "s";
    doc["protocol"] = "wifi";
    doc["detection_method"] = detection_type;
    doc["alert_level"] = "HIGH";
    doc["device_category"] = "FLOCK_SAFETY";

    doc["ssid"] = ssid;
    doc["ssid_length"] = strlen(ssid);
    doc["rssi"] = rssi;
    doc["signal_strength"] = rssi > -50 ? "STRONG" : (rssi > -70 ? "MEDIUM" : "WEAK");
    
    doc["channel"] = wifi_sniffer_get_current_channel();

    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    doc["mac_address"] = mac_str;

    char mac_prefix[9];
    snprintf(mac_prefix, sizeof(mac_prefix), "%02x:%02x:%02x", mac[0], mac[1], mac[2]);
    doc["mac_prefix"] = mac_prefix;
    doc["vendor_oui"] = mac_prefix;

    bool ssid_match = false;
    bool mac_match = false;

    for (int i = 0; i < (int)(sizeof(wifi_ssid_patterns)/sizeof(wifi_ssid_patterns[0])); i++) {
        const char* p = wifi_ssid_patterns[i];
        if (!p || !*p) continue;
        
#ifdef ENABLE_WILDCARD_VALIDATION
        if (!validate_pattern(p, PATTERN_TYPE_SSID)) {
            continue; // Ungültige Patterns überspringen
        }
#endif
        
        if (wildcard_match_ci(p, ssid)) {
            doc["matched_ssid_pattern"] = p;
            doc["ssid_match_confidence"] = "HIGH";
            ssid_match = true;
            break;
        }
    }

    for (int i = 0; i < (int)(sizeof(mac_prefixes)/sizeof(mac_prefixes[0])); i++) {
        const char* p = mac_prefixes[i];
        if (!p || !*p) continue;
        
#ifdef ENABLE_WILDCARD_VALIDATION
        if (!validate_pattern(p, PATTERN_TYPE_MAC)) {
            continue; // Ungültige Patterns überspringen
        }
#endif
        
        if (wildcard_match_mac(p, mac_str)) {
            doc["matched_mac_pattern"] = p;
            doc["mac_match_confidence"] = "HIGH";
            mac_match = true;
            break;
        }
    }

    doc["detection_criteria"] = ssid_match && mac_match ? "SSID_AND_MAC" : (ssid_match ? "SSID_ONLY" : "MAC_ONLY");
    doc["threat_score"] = ssid_match && mac_match ? 100 : (ssid_match || mac_match ? 85 : 70);

    if (strcmp(detection_type, "probe_request") == 0 || strcmp(detection_type, "probe_request_mac") == 0) {
        doc["frame_type"] = "PROBE_REQUEST";
        doc["frame_description"] = "Device actively scanning for networks";
    } else {
        doc["frame_type"] = "BEACON";
        doc["frame_description"] = "Device advertising its network";
    }

    String json_output;
    serializeJson(doc, json_output);
    Serial.println(json_output);

#if FLOCKYOU_HAS_DISPLAY
    ui_display_set_alert("WIFI", String(detection_type) + " " + String(ssid), rssi);
#endif
}

void output_ble_detection_json(const char* mac, const char* name, int rssi, const char* detection_method)
{
    DynamicJsonDocument doc(2048);

    doc["timestamp"] = millis();
    doc["detection_time"] = String(millis() / 1000.0, 3) + "s";
    doc["protocol"] = "bluetooth_le";
    doc["detection_method"] = detection_method;
    doc["alert_level"] = "HIGH";
    doc["device_category"] = "FLOCK_SAFETY";

    doc["mac_address"] = mac;
    doc["rssi"] = rssi;
    doc["signal_strength"] = rssi > -50 ? "STRONG" : (rssi > -70 ? "MEDIUM" : "WEAK");

    if (name && strlen(name) > 0) {
        doc["device_name"] = name;
        doc["device_name_length"] = strlen(name);
        doc["has_device_name"] = true;
    } else {
        doc["device_name"] = "";
        doc["device_name_length"] = 0;
        doc["has_device_name"] = false;
    }

    char mac_prefix[9];
    strncpy(mac_prefix, mac, 8);
    mac_prefix[8] = '\0';
    doc["mac_prefix"] = mac_prefix;
    doc["vendor_oui"] = mac_prefix;

    bool name_match = false;
    bool mac_match = false;

    for (int i = 0; i < (int)(sizeof(mac_prefixes)/sizeof(mac_prefixes[0])); i++) {
        const char* p = mac_prefixes[i];
        if (!p || !*p) continue;
        
#ifdef ENABLE_WILDCARD_VALIDATION
        if (!validate_pattern(p, PATTERN_TYPE_MAC)) {
            continue; // Ungültige Patterns überspringen
        }
#endif
        
        if (wildcard_match_mac(p, mac)) {
            doc["matched_mac_pattern"] = p;
            doc["mac_match_confidence"] = "HIGH";
            mac_match = true;
            break;
        }
    }

    if (name && strlen(name) > 0) {
        for (int i = 0; i < (int)(sizeof(device_name_patterns)/sizeof(device_name_patterns[0])); i++) {
            const char* p = device_name_patterns[i];
            if (!p || !*p) continue;
            
#ifdef ENABLE_WILDCARD_VALIDATION
            if (!validate_pattern(p, PATTERN_TYPE_SSID)) {
                continue; // Ungültige Patterns überspringen
            }
#endif
            
            if (wildcard_match_ci(p, name)) {
                doc["matched_name_pattern"] = p;
                doc["name_match_confidence"] = "HIGH";
                name_match = true;
                break;
            }
        }
    }

    doc["detection_criteria"] = name_match && mac_match ? "NAME_AND_MAC" : (name_match ? "NAME_ONLY" : "MAC_ONLY");
    doc["threat_score"] = name_match && mac_match ? 100 : (name_match || mac_match ? 85 : 70);

    doc["advertisement_type"] = "BLE_ADVERTISEMENT";
    doc["advertisement_description"] = "Bluetooth Low Energy device advertisement";

    if (strcmp(detection_method, "mac_prefix") == 0) {
        doc["primary_indicator"] = "MAC_ADDRESS";
        doc["detection_reason"] = "MAC address matches known Flock Safety prefix";
    } else if (strcmp(detection_method, "device_name") == 0) {
        doc["primary_indicator"] = "DEVICE_NAME";
        doc["detection_reason"] = "Device name matches Flock Safety pattern";
    }

    String json_output;
    serializeJson(doc, json_output);
    Serial.println(json_output);

#if FLOCKYOU_HAS_DISPLAY
    String line = String(detection_method) + " " + (name && strlen(name) ? String(name) : String(mac));
    ui_display_set_alert("BLE", line, rssi);
#endif
}

void output_raven_detection_json(const char* mac, int rssi, const char* service_uuid, int service_count)
{
    // Import für Service-Beschreibung
    extern const char* get_raven_service_description(const char* uuid);
    
    DynamicJsonDocument doc(2048);
    
    doc["timestamp"] = millis();
    doc["detection_time"] = String(millis() / 1000.0, 3) + "s";
    doc["protocol"] = "bluetooth_le";
    doc["detection_method"] = "raven_service_uuid";
    doc["device_type"] = "RAVEN_GUNSHOT_DETECTOR";
    doc["manufacturer"] = "SoundThinking/ShotSpotter";
    doc["alert_level"] = "CRITICAL";
    doc["mac_address"] = mac;
    doc["rssi"] = rssi;
    doc["signal_strength"] = rssi > -50 ? "STRONG" : (rssi > -70 ? "MEDIUM" : "WEAK");
    
    doc["raven_service_uuid"] = service_uuid;
    const char* service_desc = "Unknown Service";
    if (strcasecmp(service_uuid, RAVEN_DEVICE_INFO_SERVICE) == 0) service_desc = "Device Information";
    else if (strcasecmp(service_uuid, RAVEN_GPS_SERVICE) == 0) service_desc = "GPS Location";
    else if (strcasecmp(service_uuid, RAVEN_POWER_SERVICE) == 0) service_desc = "Power Management";
    else if (strcasecmp(service_uuid, RAVEN_NETWORK_SERVICE) == 0) service_desc = "Network Status";
    else if (strcasecmp(service_uuid, RAVEN_UPLOAD_SERVICE) == 0) service_desc = "Upload Statistics";
    else if (strcasecmp(service_uuid, RAVEN_ERROR_SERVICE) == 0) service_desc = "Error/Failure";
    else if (strcasecmp(service_uuid, RAVEN_OLD_HEALTH_SERVICE) == 0) service_desc = "Health (Legacy)";
    else if (strcasecmp(service_uuid, RAVEN_OLD_LOCATION_SERVICE) == 0) service_desc = "Location (Legacy)";
    
    doc["raven_service_description"] = service_desc;
    doc["raven_service_count"] = service_count;
    doc["threat_level"] = "CRITICAL";
    doc["threat_score"] = 100;

    String json_output;
    serializeJson(doc, json_output);
    Serial.println(json_output);

#if FLOCKYOU_HAS_DISPLAY
    ui_display_set_alert("RAVEN", String(service_desc), rssi);
#endif
}

void output_trigger_detection()
{
#if FLOCKYOU_NO_BUZZER
    Serial.println("DETECTION (audio disabled)");
    device_in_range = true;
    last_detection_time = millis();
    last_heartbeat = millis();
#else
    Serial.println("FLOCK SAFETY DEVICE DETECTED!");
    Serial.println("Playing alert sequence: 3 fast high-pitch beeps");
    for (int i = 0; i < 3; i++) {
        beep(DETECT_FREQ, DETECT_BEEP_DURATION);
        if (i < 2) delay(50);
    }
    Serial.println("Detection complete - device identified!\n");

    device_in_range = true;
    last_detection_time = millis();
    last_heartbeat = millis();
#endif
}

void output_heartbeat_pulse()
{
#if FLOCKYOU_NO_BUZZER
    Serial.println("Heartbeat (audio disabled)");
#else
    Serial.println("Heartbeat: Device still in range");
    beep(HEARTBEAT_FREQ, HEARTBEAT_DURATION);
    delay(100);
    beep(HEARTBEAT_FREQ, HEARTBEAT_DURATION);
#endif
}
