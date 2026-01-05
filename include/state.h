/**
 * @file state.h
 * @brief Zentrale globale State-Variablen für Flock You Detection System
 * 
 * Dieses Modul definiert alle shared state variables die von mehreren
 * Modulen (wifi_sniffer, ble_scanner, output, ui_display) verwendet werden.
 * 
 * Alle Variablen sind als extern deklariert; die tatsächlichen Definitionen
 * befinden sich in state.cpp.
 */

#pragma once

#include <Arduino.h>

// ============================================================================
// DETECTION STATE
// ============================================================================

/**
 * @brief Flag ob ein Flock Safety Gerät initial erkannt wurde
 * 
 * Wird beim ersten Detection-Event auf true gesetzt und löst
 * Audio/Visual-Feedback aus.
 */
extern bool triggered;

/**
 * @brief Flag ob aktuell ein Gerät in Reichweite ist
 * 
 * Wird auf true gesetzt bei Detection und auf false zurückgesetzt
 * wenn 30 Sekunden keine neue Detection erfolgt.
 */
extern bool device_in_range;

/**
 * @brief Zeitstempel der letzten Detektion in Millisekunden
 * 
 * Wird verwendet für Timeout-Erkennung (Out-of-Range Detection).
 */
extern unsigned long last_detection_time;

/**
 * @brief Zeitstempel des letzten Heartbeat-Pulses in Millisekunden
 * 
 * Wird verwendet für periodische Heartbeat-Audio/Visual-Signale
 * während ein Gerät in Reichweite ist.
 */
extern unsigned long last_heartbeat;
