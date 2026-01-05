/**
 * @file output.h
 * @brief Ausgabe-Funktionen für JSON-Logging und Display-Alerts
 * 
 * Dieses Modul kümmert sich um alle Ausgaben:
 * - Strukturierte JSON-Ausgabe über Serial
 * - Display-Alerts (wenn FLOCKYOU_HAS_DISPLAY aktiv)
 * - Audio-Feedback (wenn Buzzer aktiv)
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Gibt WiFi-Detektion als JSON über Serial aus
 * 
 * @param ssid Erkannte SSID
 * @param mac MAC-Adresse (6 Bytes)
 * @param rssi Signalstärke in dBm
 * @param detection_type Art der Detektion (z.B. "probe_request_unknown", "beacon_unknown")
 */
void output_wifi_detection_json(const char* ssid, const uint8_t* mac, int rssi, const char* detection_type);

/**
 * @brief Gibt BLE-Detektion als JSON über Serial aus
 * 
 * @param mac MAC-Adresse als String
 * @param name Device-Name (kann NULL sein)
 * @param rssi Signalstärke in dBm
 * @param detection_method Detektionsmethode (z.B. "unknown")
 */
void output_ble_detection_json(const char* mac, const char* name, int rssi, const char* detection_method);

/**
 * @brief Triggert Detektions-Feedback (Audio/Display)
 * 
 * Wird beim ersten Erkennen eines Geräts aufgerufen.
 * Spielt Audio-Sequenz ab (falls Buzzer aktiv) und
 * setzt interne Detektions-Flags.
 */
void output_trigger_detection();

/**
 * @brief Gibt Heartbeat-Puls aus
 * 
 * Wird periodisch aufgerufen während ein Gerät in Reichweite ist.
 */
void output_heartbeat_pulse();
