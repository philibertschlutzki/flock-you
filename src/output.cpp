/**
 * @file output.cpp
 * @brief Ausgabe-Funktionen Implementierung
 */

#include "output.h"
#include "config.h"
#include "state.h"
#include "wifi_sniffer.h"
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
    doc["device_category"] = "UNKNOWN";

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

    // Unknown device - no pattern matching in new system
    doc["detection_criteria"] = "UNKNOWN_DEVICE";
    doc["threat_score"] = 50; // Unknown devices have moderate threat score

    if (strcmp(detection_type, "probe_request_unknown") == 0) {
        doc["frame_type"] = "PROBE_REQUEST";
        doc["frame_description"] = "Unknown device actively scanning for networks";
    } else if (strcmp(detection_type, "beacon_unknown") == 0) {
        doc["frame_type"] = "BEACON";
        doc["frame_description"] = "Unknown device advertising its network";
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
    doc["device_category"] = "UNKNOWN";

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

    // Unknown device - no pattern matching in new system
    doc["detection_criteria"] = "UNKNOWN_DEVICE";
    doc["threat_score"] = 50; // Unknown devices have moderate threat score

    doc["advertisement_type"] = "BLE_ADVERTISEMENT";
    doc["advertisement_description"] = "Unknown Bluetooth Low Energy device advertisement";

    String json_output;
    serializeJson(doc, json_output);
    Serial.println(json_output);

#if FLOCKYOU_HAS_DISPLAY
    String line = String(detection_method) + " " + (name && strlen(name) ? String(name) : String(mac));
    ui_display_set_alert("BLE", line, rssi);
#endif
}

void output_trigger_detection()
{
#if FLOCKYOU_NO_BUZZER
    Serial.println("UNKNOWN DEVICE DETECTED (audio disabled)");
    device_in_range = true;
    last_detection_time = millis();
    last_heartbeat = millis();
#else
    Serial.println("UNKNOWN DEVICE DETECTED!");
    Serial.println("Playing alert sequence: 3 fast high-pitch beeps");
    for (int i = 0; i < 3; i++) {
        beep(DETECT_FREQ, DETECT_BEEP_DURATION);
        if (i < 2) delay(50);
    }
    Serial.println("Detection complete - unknown device identified!\n");

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
