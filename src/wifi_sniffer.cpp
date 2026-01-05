/**
 * @file wifi_sniffer.cpp
 * @brief WiFi Promiscuous Mode Sniffer Implementierung
 */

#define WIFI_DEBUG 1

#include "wifi_sniffer.h"
#include "config.h"
#include "blacklist.h"
#include "output.h"
#include "state.h"
#include <WiFi.h>
#include <string.h>
#include <ctype.h>

// Globale Variablen für WiFi Sniffer
static uint8_t current_channel = 1;
static unsigned long last_channel_hop = 0;

/**
 * @brief WiFi Promiscuous Mode Paket-Handler
 * 
 * Wird für jedes empfangene WiFi-Paket aufgerufen.
 * Extrahiert Informationen und prüft gegen Blacklist.
 * Nur unbekannte Geräte werden gemeldet.
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

    // Rate-limited logging to confirm WiFi activity without flooding
    static unsigned long last_rx_log = 0;
    if (millis() - last_rx_log > 2000) {
        Serial.printf("[WiFi] RX Active (Len: %d, RSSI: %d)\n", len, ppkt->rx_ctrl.rssi);
        last_rx_log = millis();
    }

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
    
    // Handle hidden/empty SSIDs
    char ssid[33] = {0};
    const char* ssid_ptr = nullptr;
    
    if (ssid_len > 0 && ssid_len <= 32 && len >= ssid_offset + 2 + ssid_len) {
        memcpy(ssid, ssid_tag + 2, ssid_len);
        ssid[ssid_len] = '\0';
        ssid_ptr = ssid;
    }

    const int rssi = ppkt->rx_ctrl.rssi;
    
    // Check blacklist - filter out known devices
    if (is_known_wifi(ssid_ptr, src_mac, bssid)) {
        // Device is known - skip (no output)
        return;
    }
    
    // Device is unknown - report it
    const char* detection_type = (frame_subtype == 4) 
        ? "probe_request_unknown"
        : "beacon_unknown";

    if (!triggered) {
        output_trigger_detection();
        triggered = true;
    }

    output_wifi_detection_json(ssid_ptr ? ssid_ptr : "", src_mac, rssi, detection_type);
}

void wifi_sniffer_init()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
    esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);

    Serial.println("[WiFi] Sniffer INITIALIZED - Monitor Mode Active");
    Serial.printf("WiFi promiscuous mode enabled on channel %d\n", current_channel);
    Serial.println("Monitoring probe requests and beacons (unknown-only mode)...");
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
        Serial.printf("[WiFi] Channel Hop -> %d\n", current_channel);
        last_channel_hop = now;
    }
}

uint8_t wifi_sniffer_get_current_channel()
{
    return current_channel;
}
