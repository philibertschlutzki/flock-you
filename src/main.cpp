/**
 * @file main.cpp
 * @brief Hauptprogramm für Flock You Detection System (Orchestrator)
 * 
 * Dieses Programm koordiniert alle Module des Flock You Detection Systems.
 * Es delegiert die eigentliche Implementierung an spezialisierte Module:
 * - wifi_sniffer: WiFi Promiscuous Mode & Channel Hopping
 * - ble_scanner: BLE Scanning & Device Detection
 * - output: JSON-Ausgabe & Audio/Display-Feedback
 * - ui_display: TFT Display Rendering
 * - diagnostics: Hardware-Selbsttest
 * 
 * main.cpp ist NUR für Orchestrierung zuständig:
 * - Initialisierung aller Module
 * - Koordination der Loop-Aufrufe
 * - Out-of-Range Timeout-Management
 * - Heartbeat-Timing
 * 
 * Hardware-Anforderungen:
 * - LILYGO T-Display-S3 (ESP32-S3 + ST7789 1.9" Display)
 * - USB-C Stromversorgung
 * - Optional: Buttons für Diagnostics
 * 
 * Compile-Time Flags:
 * - FLOCKYOU_HAS_DISPLAY=1: Aktiviert Display UI
 * - FLOCKYOU_NO_BUZZER=1: Deaktiviert Audio (T-Display-S3 Standard)
 * 
 * @author Flock You Team
 * @version 0.2.0
 * @date 2026-01-05
 * 
 * @see wifi_sniffer.h für WiFi-Funktionalität
 * @see ble_scanner.h für BLE-Funktionalität
 * @see output.h für JSON-Ausgabe
 * @see ui_display.h für Display-UI
 * @see diagnostics.h für Hardware-Selbsttest
 * @see state.h für globale State-Variablen
 */

#include <Arduino.h>
#include "config.h"
#include "state.h"
#include "wifi_sniffer.h"
#include "ble_scanner.h"
#include "output.h"
#include "diagnostics.h"
#include "pattern_startup_check.h"

#if FLOCKYOU_HAS_DISPLAY
#include "ui_display.h"
#include <TFT_eSPI.h>
// Global TFT instance - nur für diagnostics.cpp Zugriff
TFT_eSPI tft = TFT_eSPI();
#endif

// ============================================================================
// SETUP - Initialisierung aller Module
// ============================================================================

/**
 * @brief Arduino Setup-Funktion - Orchestriert die Initialisierung
 * 
 * Initialisiert alle Module in der korrekten Reihenfolge:
 * 1. Serial Port (115200 Baud)
 * 2. Hardware Diagnostics Check (Button A Hold Detection)
 * 3. Display UI (falls FLOCKYOU_HAS_DISPLAY=1)
 * 4. WiFi Promiscuous Mode Sniffer
 * 5. BLE Scanner (NimBLE)
 * 
 * Diagnostics Mode:
 * Falls Button A beim Boot für 3 Sekunden gedrückt gehalten wird,
 * startet der Hardware-Selbsttest statt des normalen Betriebs.
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
        #ifdef TFT_BL
        pinMode(TFT_BL, OUTPUT);
        #ifdef TFT_BACKLIGHT_ON
        digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
        #else
        digitalWrite(TFT_BL, HIGH);
        #endif
        #endif
        #endif
        
        // Starte Diagnostics (kehrt nie zurück)
        diagnostics_run_all_tests();
    }
    // ========================================================================

    Serial.println("Starting Flock You Detection System...\n");

    // Validiere alle Patterns beim Boot
#ifdef ENABLE_WILDCARD_VALIDATION
    validate_all_patterns();
#endif

    // Initialisiere Display
    #if FLOCKYOU_HAS_DISPLAY
    ui_display_init();
    #endif

    // Initialisiere WiFi Sniffer
    wifi_sniffer_init();

    // Initialisiere BLE Scanner
    ble_scanner_init();

    Serial.println("System ready - hunting for Flock Safety devices...\n");
}

// ============================================================================
// LOOP - Hauptschleife (Orchestrierung)
// ============================================================================

/**
 * @brief Arduino Loop-Funktion - Koordiniert alle Module
 * 
 * Diese Funktion läuft kontinuierlich und ruft die Tick-Funktionen
 * aller Module auf:
 * 
 * 1. WiFi Channel Hopping (wifi_sniffer_hop_channel)
 * 2. Heartbeat Management (output_heartbeat_pulse alle 10s)
 * 3. Out-of-Range Detection (nach 30s ohne neue Detection)
 * 4. BLE Scan Management (ble_scanner_tick)
 * 5. Display UI Update (ui_display_tick, alle 200ms intern)
 * 
 * Timing:
 * - WiFi Channel Hop: alle 500ms (intern in wifi_sniffer)
 * - Heartbeat: alle 10s (nur wenn device_in_range)
 * - Out-of-Range Timeout: 30s nach letzter Detection
 * - BLE Scan: 1s Scan alle 5s (intern in ble_scanner)
 * - Display Update: alle 200ms (intern in ui_display)
 * - Loop Delay: 100ms
 * 
 * @note Diese Funktion läuft endlos, nur unterbrochen durch Reset/Power-Off
 */
void loop()
{
    // WiFi Channel Hopping
    wifi_sniffer_hop_channel();

    // Heartbeat & Out-of-Range Management
    if (device_in_range) {
        unsigned long now = millis();

        // Heartbeat alle 10 Sekunden
        if (now - last_heartbeat >= 10000) {
            output_heartbeat_pulse();
            last_heartbeat = now;
        }

        // Out-of-Range Timeout nach 30 Sekunden
        if (now - last_detection_time >= 30000) {
            Serial.println("Device out of range - stopping heartbeat");
            device_in_range = false;
            triggered = false;
        }
    }

    // BLE Scan Tick
    ble_scanner_tick();

    // Display Update
    #if FLOCKYOU_HAS_DISPLAY
    ui_display_tick(ble_scanner_is_scanning());
    #endif

    delay(100);
}
