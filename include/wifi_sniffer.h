/**
 * @file wifi_sniffer.h
 * @brief WiFi Promiscuous Mode Sniffer für Unknown Device Detection
 * 
 * Dieses Modul implementiert WiFi-Überwachung im Promiscuous Mode,
 * Channel Hopping und Detektion von unbekannten Geräten.
 * Bekannte Geräte (Blacklist) werden gefiltert.
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
