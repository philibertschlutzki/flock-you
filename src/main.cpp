/**
 * @file main.cpp
 * @brief Hauptprogramm für Flock You Detection System
 * 
 * Dieses Programm implementiert ein Multi-Protocol Überwachungsgerät-Erkennungssystem
 * für das LILYGO T-Display-S3 Board. Es kombiniert WiFi Promiscuous Mode Sniffing
 * und BLE Scanning um Flock Safety Kameras und Raven Schusserkennungsgeräte zu
 * identifizieren.
 * 
 * Hardware-Anforderungen:
 * - LILYGO T-Display-S3 (ESP32-S3 + ST7789 1.9" Display)
 * - USB-C Stromversorgung
 * - Optional: Buttons für Diagnostics
 * 
 * Features:
 * - WiFi Promiscuous Mode mit Channel Hopping (Kanal 1-13)
 * - BLE Active Scanning mit NimBLE
 * - Display UI (Status + Alert Screens)
 * - Hardware Diagnostics Mode (Button Hold beim Boot)
 * - JSON Serial Output für alle Detections
 * - SSID, MAC und BLE Service UUID Matching
 * 
 * Erkennungsmethoden:
 * 1. WiFi SSID Pattern Matching (z.B. "Flock", "FS Ext Battery")
 * 2. WiFi MAC-Präfix Matching (bekannte Hersteller-OUIs)
 * 3. BLE Device Name Pattern Matching
 * 4. BLE Service UUID Detection (Raven-spezifische UUIDs)
 * 
 * Compile-Time Flags:
 * - FLOCKYOU_HAS_DISPLAY=1: Aktiviert Display UI
 * - FLOCKYOU_NO_BUZZER=1: Deaktiviert Audio (T-Display-S3 Standard)
 * 
 * @author Flock You Team
 * @version 0.1.0
 * @date 2026-01-04
 * 
 * @see diagnostics.h für Hardware-Selbsttest
 * @see config.h für Pin-Konfiguration
 */

#include <Arduino.h>
#include <WiFi.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <ArduinoJson.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "diagnostics.h"

#ifndef FLOCKYOU_NO_BUZZER
#define FLOCKYOU_NO_BUZZER 0
#endif

#ifndef FLOCKYOU_HAS_DISPLAY
#define FLOCKYOU_HAS_DISPLAY 0
#endif

#if FLOCKYOU_HAS_DISPLAY
#include <TFT_eSPI.h>
// Global TFT instance (changed from static to enable access from diagnostics.cpp for hardware testing)
TFT_eSPI tft = TFT_eSPI();
static unsigned long last_ui_render = 0;
static unsigned long last_alert_until = 0;
static String last_alert_title;
static String last_alert_line;
static int last_alert_rssi = 0;
static const unsigned long UI_RENDER_INTERVAL_MS = 200;
static const unsigned long ALERT_DISPLAY_MS = 8000;
#endif

// ============================================================================
// CONFIGURATION
// ============================================================================

// Audio Configuration (nicht verwendet auf LILYGO T-Display-S3)
#define LOW_FREQ 200      // Boot sequence - low pitch
#define HIGH_FREQ 800     // Boot sequence - high pitch & detection alert
#define DETECT_FREQ 1000  // Detection alert - high pitch (faster beeps)
#define HEARTBEAT_FREQ 600 // Heartbeat pulse frequency
#define BOOT_BEEP_DURATION 300   // Boot beep duration
#define DETECT_BEEP_DURATION 150 // Detection beep duration (faster)
#define HEARTBEAT_DURATION 100   // Short heartbeat pulse

// WiFi Promiscuous Mode Configuration
#define MAX_CHANNEL 13
#define CHANNEL_HOP_INTERVAL 500  // milliseconds

// BLE SCANNING CONFIGURATION
#define BLE_SCAN_DURATION 1    // Seconds
#define BLE_SCAN_INTERVAL 5000 // Milliseconds between scans
static unsigned long last_ble_scan = 0;

// Detection Pattern Limits
#define MAX_SSID_PATTERNS 10
#define MAX_MAC_PATTERNS 50
#define MAX_DEVICE_NAMES 20

// ============================================================================
// DETECTION PATTERNS (Extracted from Real Flock Safety Device Databases)
// ============================================================================

static const char* wifi_ssid_patterns[] = {
    "flock",
    "Flock",
    "FLOCK",
    "FS Ext Battery",
    "Penguin",
    "Pigvision"
};

static const char* mac_prefixes[] = {
    "58:8e:81", "cc:cc:cc", "ec:1b:bd", "90:35:ea", "04:0d:84",
    "f0:82:c0", "1c:34:f1", "38:5b:44", "94:34:69", "b4:e3:f9",

    "70:c9:4e", "3c:91:80", "d8:f3:bc", "80:30:49", "14:5a:fc",
    "74:4c:a1", "08:3a:88", "9c:2f:9d", "94:08:53", "e4:aa:ea"
};

static const char* device_name_patterns[] = {
    "FS Ext Battery",
    "Penguin",
    "Flock",
    "Pigvision"
};

// ============================================================================
// RAVEN SURVEILLANCE DEVICE UUID PATTERNS
// ============================================================================

#define RAVEN_DEVICE_INFO_SERVICE       "0000180a-0000-1000-8000-00805f9b34fb"
#define RAVEN_GPS_SERVICE               "00003100-0000-1000-8000-00805f9b34fb"
#define RAVEN_POWER_SERVICE             "00003200-0000-1000-8000-00805f9b34fb"
#define RAVEN_NETWORK_SERVICE           "00003300-0000-1000-8000-00805f9b34fb"
#define RAVEN_UPLOAD_SERVICE            "00003400-0000-1000-8000-00805f9b34fb"
#define RAVEN_ERROR_SERVICE             "00003500-0000-1000-8000-00805f9b34fb"
#define RAVEN_OLD_HEALTH_SERVICE        "00001809-0000-1000-8000-00805f9b34fb"
#define RAVEN_OLD_LOCATION_SERVICE      "00001819-0000-1000-8000-00805f9b34fb"

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

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static uint8_t current_channel = 1;
static unsigned long last_channel_hop = 0;
static bool triggered = false;
static bool device_in_range = false;
static unsigned long last_detection_time = 0;
static unsigned long last_heartbeat = 0;
static NimBLEScan* pBLEScan;

// ============================================================================
// DISPLAY UI (LILYGO T-Display-S3 build)
// ============================================================================

#if FLOCKYOU_HAS_DISPLAY
/**
 * @brief Initialisiert das TFT Display
 * 
 * Konfiguriert TFT_eSPI, setzt Rotation auf Landscape (1),
 * aktiviert Backlight und zeigt einen kurzen Splash Screen.
 */
static void display_init()
{
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    tft.drawString("Flock You", 4, 4, 4);
    tft.drawString("Display-only", 4, 28, 2);
}

/**
 * @brief Setzt Alert-Informationen für Display-Anzeige
 * 
 * Speichert Alert-Daten und aktiviert Alert-Screen für 8 Sekunden.
 * Nach Ablauf der Zeit wechselt Display automatisch zurück zum Status-Screen.
 * 
 * @param title Alert-Typ (z.B. "WIFI", "BLE", "RAVEN")
 * @param line Detail-Zeile (z.B. SSID, Device-Name)
 * @param rssi Signalstärke in dBm
 */
static void display_set_alert(const String& title, const String& line, int rssi)
{
    last_alert_title = title;
    last_alert_line = line;
    last_alert_rssi = rssi;
    last_alert_until = millis() + ALERT_DISPLAY_MS;
}

static void display_render_alert()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("ALERT", 4, 4, 4);

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(last_alert_title, 4, 40, 2);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(last_alert_line, 4, 62, 2);

    char buf[32];
    snprintf(buf, sizeof(buf), "RSSI: %d dBm", last_alert_rssi);
    tft.drawString(buf, 4, 86, 2);
}

static void display_render_status(bool ble_scanning)
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Flock You", 4, 4, 4);

    char buf[64];
    snprintf(buf, sizeof(buf), "WiFi CH: %u", current_channel);
    tft.drawString(buf, 4, 36, 2);

    snprintf(buf, sizeof(buf), "BLE: %s", ble_scanning ? "scanning" : "idle");
    tft.drawString(buf, 4, 56, 2);

    snprintf(buf, sizeof(buf), "In range: %s", device_in_range ? "YES" : "NO");
    tft.drawString(buf, 4, 76, 2);

    float up = millis() / 1000.0f;
    snprintf(buf, sizeof(buf), "Uptime: %.1fs", up);
    tft.drawString(buf, 4, 96, 2);
}

static void display_tick(bool ble_scanning)
{
    unsigned long now = millis();
    if (now - last_ui_render < UI_RENDER_INTERVAL_MS) return;
    last_ui_render = now;

    if (last_alert_until > now) {
        display_render_alert();
    } else {
        display_render_status(ble_scanning);
    }
}
#endif

// ============================================================================
// AUDIO SYSTEM
// ============================================================================

static void boot_beep_sequence()
{
#if FLOCKYOU_NO_BUZZER
    printf("Audio disabled (FLOCKYOU_NO_BUZZER)\n");
#else
    printf("Initializing audio system...\n");
    printf("Playing boot sequence: Low -> High pitch\n");
    beep(LOW_FREQ, BOOT_BEEP_DURATION);
    beep(HIGH_FREQ, BOOT_BEEP_DURATION);
    printf("Audio system ready\n\n");
#endif
}

static void flock_detected_beep_sequence()
{
#if FLOCKYOU_NO_BUZZER
    printf("DETECTION (audio disabled)\n");
    device_in_range = true;
    last_detection_time = millis();
    last_heartbeat = millis();
#else
    printf("FLOCK SAFETY DEVICE DETECTED!\n");
    printf("Playing alert sequence: 3 fast high-pitch beeps\n");
    for (int i = 0; i < 3; i++) {
        beep(DETECT_FREQ, DETECT_BEEP_DURATION);
        if (i < 2) delay(50);
    }
    printf("Detection complete - device identified!\n\n");

    device_in_range = true;
    last_detection_time = millis();
    last_heartbeat = millis();
#endif
}

static void heartbeat_pulse()
{
#if FLOCKYOU_NO_BUZZER
    printf("Heartbeat (audio disabled)\n");
#else
    printf("Heartbeat: Device still in range\n");
    beep(HEARTBEAT_FREQ, HEARTBEAT_DURATION);
    delay(100);
    beep(HEARTBEAT_FREQ, HEARTBEAT_DURATION);
#endif
}

// ============================================================================
// JSON OUTPUT FUNCTIONS
// ============================================================================

static void output_wifi_detection_json(const char* ssid, const uint8_t* mac, int rssi, const char* detection_type)
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
    doc["channel"] = current_channel;

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
        if (strcasestr(ssid, wifi_ssid_patterns[i])) {
            doc["matched_ssid_pattern"] = wifi_ssid_patterns[i];
            doc["ssid_match_confidence"] = "HIGH";
            ssid_match = true;
            break;
        }
    }

    for (int i = 0; i < (int)(sizeof(mac_prefixes)/sizeof(mac_prefixes[0])); i++) {
        if (strncasecmp(mac_prefix, mac_prefixes[i], 8) == 0) {
            doc["matched_mac_pattern"] = mac_prefixes[i];
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
    display_set_alert("WIFI", String(detection_type) + " " + String(ssid), rssi);
#endif
}

static void output_ble_detection_json(const char* mac, const char* name, int rssi, const char* detection_method)
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
        if (strncasecmp(mac, mac_prefixes[i], strlen(mac_prefixes[i])) == 0) {
            doc["matched_mac_pattern"] = mac_prefixes[i];
            doc["mac_match_confidence"] = "HIGH";
            mac_match = true;
            break;
        }
    }

    if (name && strlen(name) > 0) {
        for (int i = 0; i < (int)(sizeof(device_name_patterns)/sizeof(device_name_patterns[0])); i++) {
            if (strcasestr(name, device_name_patterns[i])) {
                doc["matched_name_pattern"] = device_name_patterns[i];
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
    display_set_alert("BLE", line, rssi);
#endif
}

// ============================================================================
// DETECTION HELPER FUNCTIONS
// ============================================================================

static bool check_mac_prefix(const uint8_t* mac)
{
    char mac_str[9];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x", mac[0], mac[1], mac[2]);

    for (int i = 0; i < (int)(sizeof(mac_prefixes)/sizeof(mac_prefixes[0])); i++) {
        if (strncasecmp(mac_str, mac_prefixes[i], 8) == 0) {
            return true;
        }
    }
    return false;
}

static bool check_ssid_pattern(const char* ssid)
{
    if (!ssid) return false;

    for (int i = 0; i < (int)(sizeof(wifi_ssid_patterns)/sizeof(wifi_ssid_patterns[0])); i++) {
        if (strcasestr(ssid, wifi_ssid_patterns[i])) {
            return true;
        }
    }
    return false;
}

static bool check_device_name_pattern(const char* name)
{
    if (!name) return false;

    for (int i = 0; i < (int)(sizeof(device_name_patterns)/sizeof(device_name_patterns[0])); i++) {
        if (strcasestr(name, device_name_patterns[i])) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// RAVEN UUID DETECTION
// ============================================================================

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
            if (strcasecmp(uuidStr.c_str(), raven_service_uuids[j]) == 0) {
                if (detected_service_out != nullptr) {
                    strncpy(detected_service_out, uuidStr.c_str(), 40);
                }
                return true;
            }
        }
    }

    return false;
}

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

// ============================================================================
// WIFI PROMISCUOUS MODE HANDLER
// ============================================================================

typedef struct {
    unsigned frame_ctrl:16;
    unsigned duration_id:16;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    unsigned sequence_ctrl:16;
    uint8_t addr4[6];
} wifi_ieee80211_mac_hdr_t;

typedef struct {
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[0];
} wifi_ieee80211_packet_t;

static void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{
    (void)type;

    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
    const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

    uint8_t frame_type = (hdr->frame_ctrl & 0xFF) >> 2;
    if (frame_type != 0x20 && frame_type != 0x80) {
        return;
    }

    char ssid[33] = {0};
    uint8_t *payload = (uint8_t *)ipkt + 24;

    if (frame_type == 0x80) {
        payload += 12;
    }

    if (payload[0] == 0 && payload[1] <= 32) {
        memcpy(ssid, &payload[2], payload[1]);
        ssid[payload[1]] = '\0';
    }

    if (strlen(ssid) > 0 && check_ssid_pattern(ssid)) {
        const char* detection_type = (frame_type == 0x20) ? "probe_request" : "beacon";
        output_wifi_detection_json(ssid, hdr->addr2, ppkt->rx_ctrl.rssi, detection_type);

        if (!triggered) {
            triggered = true;
            flock_detected_beep_sequence();
        }
        last_detection_time = millis();
        return;
    }

    if (check_mac_prefix(hdr->addr2)) {
        const char* detection_type = (frame_type == 0x20) ? "probe_request_mac" : "beacon_mac";
        output_wifi_detection_json(ssid[0] ? ssid : "hidden", hdr->addr2, ppkt->rx_ctrl.rssi, detection_type);

        if (!triggered) {
            triggered = true;
            flock_detected_beep_sequence();
        }
        last_detection_time = millis();
        return;
    }
}

// ============================================================================
// BLE SCANNING
// ============================================================================

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

        if (check_mac_prefix(mac)) {
            output_ble_detection_json(addrStr.c_str(), name.c_str(), rssi, "mac_prefix");
            if (!triggered) {
                triggered = true;
                flock_detected_beep_sequence();
            }
            last_detection_time = millis();
            return;
        }

        if (!name.empty() && check_device_name_pattern(name.c_str())) {
            output_ble_detection_json(addrStr.c_str(), name.c_str(), rssi, "device_name");
            if (!triggered) {
                triggered = true;
                flock_detected_beep_sequence();
            }
            last_detection_time = millis();
            return;
        }

        char detected_service_uuid[41] = {0};
        if (check_raven_service_uuid(advertisedDevice, detected_service_uuid)) {
            const char* fw_version = estimate_raven_firmware_version(advertisedDevice);
            const char* service_desc = get_raven_service_description(detected_service_uuid);

            StaticJsonDocument<1024> doc;
            doc["protocol"] = "bluetooth_le";
            doc["detection_method"] = "raven_service_uuid";
            doc["device_type"] = "RAVEN_GUNSHOT_DETECTOR";
            doc["manufacturer"] = "SoundThinking/ShotSpotter";
            doc["mac_address"] = addrStr.c_str();
            doc["rssi"] = rssi;
            doc["signal_strength"] = rssi > -50 ? "STRONG" : (rssi > -70 ? "MEDIUM" : "WEAK");

            if (!name.empty()) {
                doc["device_name"] = name.c_str();
            }

            doc["raven_service_uuid"] = detected_service_uuid;
            doc["raven_service_description"] = service_desc;
            doc["raven_firmware_version"] = fw_version;
            doc["threat_level"] = "CRITICAL";
            doc["threat_score"] = 100;

            if (advertisedDevice->haveServiceUUID()) {
                JsonArray services = doc.createNestedArray("service_uuids");
                int serviceCount = advertisedDevice->getServiceUUIDCount();
                for (int i = 0; i < serviceCount; i++) {
                    NimBLEUUID serviceUUID = advertisedDevice->getServiceUUID(i);
                    services.add(serviceUUID.toString().c_str());
                }
            }

            serializeJson(doc, Serial);
            Serial.println();

#if FLOCKYOU_HAS_DISPLAY
            display_set_alert("RAVEN", String(service_desc) + " " + String(fw_version), rssi);
#endif

            if (!triggered) {
                triggered = true;
                flock_detected_beep_sequence();
            }
            last_detection_time = millis();
            return;
        }
    }
};

// ============================================================================
// CHANNEL HOPPING
// ============================================================================

static void hop_channel()
{
    unsigned long now = millis();
    if (now - last_channel_hop > CHANNEL_HOP_INTERVAL) {
        current_channel++;
        if (current_channel > MAX_CHANNEL) {
            current_channel = 1;
        }
        esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
        last_channel_hop = now;
        printf("[WiFi] Hopped to channel %d\n", current_channel);
    }
}

// ============================================================================
// MAIN FUNCTIONS
// ============================================================================

/**
 * @brief Arduino Setup-Funktion - Initialisierung aller Komponenten
 * 
 * Diese Funktion wird einmal beim Boot ausgeführt und initialisiert:
 * 
 * 1. Serial Port (115200 Baud)
 * 2. Hardware Diagnostics Check (Button A Hold Detection)
 * 3. Audio System (falls BUZZER vorhanden)
 * 4. Display UI (falls DISPLAY vorhanden)
 * 5. WiFi Promiscuous Mode
 * 6. BLE Scanner (NimBLE)
 * 
 * Diagnostics Mode:
 * Falls Button A beim Boot für 3 Sekunden gedrückt gehalten wird,
 * startet der Hardware-Selbsttest statt des normalen Betriebs.
 * (siehe diagnostics.h für Details)
 * 
 * @note Diese Funktion kehrt zurück, außer wenn Diagnostics-Modus aktiviert wird
 */
void setup()
{
    Serial.begin(115200);
    delay(1000);

    // ========================================================================
    // DIAGNOSTICS MODE CHECK
    // ========================================================================
    // Prüfe ob Diagnostics-Modus aktiviert werden soll (Button A beim Boot)
    if (diagnostics_should_enter()) {
        #if FLOCKYOU_HAS_DISPLAY
        // Initialisiere Display für Diagnostics
        tft.init();
        tft.setRotation(1);
        tft.fillScreen(TFT_BLACK);
        #ifdef TFT_BACKLIGHT_PIN
        pinMode(TFT_BACKLIGHT_PIN, OUTPUT);
        #ifdef TFT_BACKLIGHT_ON
        digitalWrite(TFT_BACKLIGHT_PIN, TFT_BACKLIGHT_ON);
        #else
        digitalWrite(TFT_BACKLIGHT_PIN, HIGH);
        #endif
        #endif
        #endif
        
        // Starte Diagnostics (kehrt nie zurück)
        diagnostics_run_all_tests();
    }
    // ========================================================================

    boot_beep_sequence();

#if FLOCKYOU_HAS_DISPLAY
    display_init();
#endif

    printf("Starting Flock Squawk Enhanced Detection System...\n\n");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
    esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);

    printf("WiFi promiscuous mode enabled on channel %d\n", current_channel);
    printf("Monitoring probe requests and beacons...\n");

    printf("Initializing BLE scanner...\n");
    NimBLEDevice::init("");
    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    printf("BLE scanner initialized\n");
    printf("System ready - hunting for Flock Safety devices...\n\n");

    last_channel_hop = millis();
}

/**
 * @brief Arduino Loop-Funktion - Hauptschleife
 * 
 * Diese Funktion läuft kontinuierlich und führt folgende Tasks aus:
 * 
 * 1. WiFi Channel Hopping (alle 500ms zum nächsten Kanal)
 * 2. Heartbeat Management (Pieptöne alle 10s wenn Gerät in Reichweite)
 * 3. Out-of-Range Detection (nach 30s ohne neue Detection)
 * 4. BLE Scan Management (periodische 1s Scans alle 5s)
 * 5. Display UI Update (falls DISPLAY vorhanden, alle 200ms)
 * 
 * Timing:
 * - Channel Hop: alle 500ms
 * - Heartbeat: alle 10s (nur wenn device_in_range)
 * - Out-of-Range Timeout: 30s nach letzter Detection
 * - BLE Scan: 1s Scan alle 5s
 * - Display Update: alle 200ms
 * - Loop Delay: 100ms
 * 
 * @note Diese Funktion läuft endlos, nur unterbrochen durch Reset/Power-Off
 */
void loop()
{
    hop_channel();

    if (device_in_range) {
        unsigned long now = millis();

        if (now - last_heartbeat >= 10000) {
            heartbeat_pulse();
            last_heartbeat = now;
        }

        if (now - last_detection_time >= 30000) {
            printf("Device out of range - stopping heartbeat\n");
            device_in_range = false;
            triggered = false;
        }
    }

    if (millis() - last_ble_scan >= BLE_SCAN_INTERVAL && !pBLEScan->isScanning()) {
        printf("[BLE] scan...\n");
        pBLEScan->start(BLE_SCAN_DURATION, false);
        last_ble_scan = millis();
    }

    if (!pBLEScan->isScanning() && millis() - last_ble_scan > BLE_SCAN_DURATION * 1000) {
        pBLEScan->clearResults();
    }

#if FLOCKYOU_HAS_DISPLAY
    display_tick(pBLEScan && pBLEScan->isScanning());
#endif

    delay(100);
}
