/**
 * @file wifi_sniffer.h
 * @brief WiFi Promiscuous Mode Sniffer für Flock Safety Detection
 * 
 * Dieses Modul implementiert WiFi-Überwachung im Promiscuous Mode,
 * Channel Hopping und Detektion von Flock Safety Geräten anhand von
 * SSID-Mustern und MAC-Adressen-Präfixen.
 */

#pragma once

#include <Arduino.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"

/**
 * @brief Initialisiert den WiFi Sniffer
 * 
 * Setzt WiFi in Station-Mode, aktiviert Promiscuous Mode
 * und registriert Paket-Handler.
 */
void wifi_sniffer_init();

/**
 * @brief Führt Channel-Hopping durch
 * 
 * Wechselt periodisch zwischen WiFi-Kanälen basierend auf
 * konfiguriertem CHANNEL_HOP_INTERVAL.
 */
void wifi_sniffer_hop_channel();

/**
 * @brief Gibt aktuellen WiFi-Kanal zurück
 * @return Aktuelle Kanalnummer (1-13)
 */
uint8_t wifi_sniffer_get_current_channel();

/**
 * @brief Prüft ob SSID einem bekannten Flock Safety Muster entspricht
 * @param ssid Die zu prüfende SSID
 * @return true wenn SSID-Muster übereinstimmt, sonst false
 */
bool wifi_check_ssid_pattern(const char* ssid);

/**
 * @brief Prüft ob MAC-Adresse einem bekannten Flock Safety Präfix entspricht
 * @param mac MAC-Adresse (6 Bytes)
 * @return true wenn MAC-Präfix übereinstimmt, sonst false
 */
bool wifi_check_mac_prefix(const uint8_t* mac);
