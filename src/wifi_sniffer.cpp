/**
 * @file wifi_sniffer.cpp
 * @brief WiFi Promiscuous Mode Sniffer Implementierung
 */

#include "wifi_sniffer.h"
#include "config.h"
#include "detection_patterns.h"
#include "output.h"
#include <WiFi.h>
#include <string.h>
#include <ctype.h>

// Globale Variablen für WiFi Sniffer
static uint8_t current_channel = 1;
static unsigned long last_channel_hop = 0;
extern bool triggered;
extern bool device_in_range;
extern unsigned long last_detection_time;

/**
 * @brief WiFi Promiscuous Mode Paket-Handler
 * 
 * Wird für jedes empfangene WiFi-Paket aufgerufen.
 * Extrahiert Informationen und prüft auf Flock Safety Geräte.
 * 
 * @param buf Rohe Paketdaten
 * @param type Pakettyp
 */
static void wifi_sniffer_packet_handler(void* buf, wifi_promiscuous_pkt_type_t type)
{
    if (type != WIFI_PKT_MGMT) return;

    const wifi_promiscuous_pkt_t* ppkt = (wifi_promiscuous_pkt_t*)buf;
    const uint8_t* payload = ppkt->payload;
    const int len = ppkt->rx_ctrl.sig_len;

    if (len < 36) return;

    const uint8_t frame_type = payload[0];
    const uint8_t frame_subtype = (payload[0] & 0xF0) >> 4;

    // Probe Request (Subtype 4) oder Beacon (Subtype 8)
    if (frame_subtype != 4 && frame_subtype != 8) return;

    const uint8_t* src_mac = payload + 10;
    const uint8_t* bssid = (frame_subtype == 4) ? (payload + 10) : (payload + 16);

    // SSID extrahieren
    int ssid_offset = (frame_subtype == 4) ? 24 : 36;
    if (len < ssid_offset + 2) return;

    const uint8_t* ssid_tag = payload + ssid_offset;
    if (ssid_tag[0] != 0x00) return;

    const uint8_t ssid_len = ssid_tag[1];
    if (ssid_len == 0 || ssid_len > 32 || len < ssid_offset + 2 + ssid_len) return;

    char ssid[33];
    memcpy(ssid, ssid_tag + 2, ssid_len);
    ssid[ssid_len] = '\0';

    const int rssi = ppkt->rx_ctrl.rssi;
    const bool ssid_match = wifi_check_ssid_pattern(ssid);
    const bool mac_match = wifi_check_mac_prefix(src_mac);

    if (ssid_match || mac_match) {
        const char* detection_type = (frame_subtype == 4) 
            ? (ssid_match ? "probe_request" : "probe_request_mac")
            : (ssid_match ? "beacon" : "beacon_mac");

        if (!triggered) {
            output_trigger_detection();
            triggered = true;
        }

        output_wifi_detection_json(ssid, src_mac, rssi, detection_type);
    }
}

void wifi_sniffer_init()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
    esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);

    Serial.printf("WiFi promiscuous mode enabled on channel %d\n", current_channel);
    Serial.println("Monitoring probe requests and beacons...");
}

void wifi_sniffer_hop_channel()
{
    unsigned long now = millis();
    if (now - last_channel_hop >= CHANNEL_HOP_INTERVAL) {
        current_channel++;
        if (current_channel > MAX_CHANNEL) {
            current_channel = 1;
        }
        esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
        last_channel_hop = now;
    }
}

uint8_t wifi_sniffer_get_current_channel()
{
    return current_channel;
}

bool wifi_check_ssid_pattern(const char* ssid)
{
    if (!ssid) return false;

    for (int i = 0; i < (int)(sizeof(wifi_ssid_patterns)/sizeof(wifi_ssid_patterns[0])); i++) {
        if (strcasestr(ssid, wifi_ssid_patterns[i])) {
            return true;
        }
    }
    return false;
}

bool wifi_check_mac_prefix(const uint8_t* mac)
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
