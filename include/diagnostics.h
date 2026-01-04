/**
 * @file diagnostics.h
 * @brief Hardware-Selbsttest und Diagnose-Modus für LILYGO T-Display-S3
 * 
 * Dieser Diagnose-Modus testet alle relevanten Hardware-Komponenten:
 * - Display (ST7789): Farbflächen, Text, Rotation
 * - Backlight: On/Off, PWM Ramping (falls verfügbar)
 * - Buttons: A/B Live-Anzeige, Debouncing
 * - WiFi: Initialisierung, Channel Hopping, Frame-Empfang
 * - BLE: Scan, Geräte-Zählung, letztes Gerät
 * - PSRAM/Heap: Speicher-Test
 * 
 * Aktivierung: Button A beim Boot gedrückt halten (3 Sekunden)
 * 
 * @author Flock You Team
 * @date 2026-01-04
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Überprüft ob Diagnostics-Modus aktiviert werden soll
 * 
 * Liest Button A beim Boot und prüft ob er für DIAGNOSTICS_BOOT_HOLD_MS
 * gedrückt gehalten wird.
 * 
 * @return true wenn Diagnostics-Modus aktiviert werden soll
 * @return false für normalen Betrieb
 */
bool diagnostics_should_enter();

/**
 * @brief Führt vollständigen Hardware-Selbsttest durch
 * 
 * Diese Funktion startet eine interaktive Test-Sequenz die alle
 * Hardware-Komponenten überprüft und die Ergebnisse sowohl auf
 * dem Display als auch per Serial (JSON) ausgibt.
 * 
 * Die Funktion läuft in einer Endlosschleife und kehrt nicht zurück.
 * Nach Abschluss aller Tests kann das Board per Reset neu gestartet werden.
 * 
 * Tests:
 * - Display: Farbflächen (RGB, Schwarz, Weiß), Text, Rotation
 * - Backlight: On/Off Toggle, PWM Ramping
 * - Buttons: Live-Anzeige, Debounce-Test
 * - WiFi: Init, Channel Hopping, Frame Counter
 * - BLE: Scan, Device Counter, Last Device Info
 * - Memory: PSRAM Check, Heap Status
 * 
 * @note Diese Funktion kehrt niemals zurück (Endlosschleife)
 */
void diagnostics_run_all_tests();

/**
 * @brief Test 1: Display Farb- und Text-Test
 * 
 * Zeigt verschiedene Farbflächen und Texte an um zu verifizieren
 * dass das Display korrekt initialisiert ist und alle Farben
 * darstellen kann.
 * 
 * @return true wenn Test erfolgreich
 */
bool diagnostics_test_display();

/**
 * @brief Test 2: Backlight On/Off und PWM Ramping
 * 
 * Testet die Backlight-Steuerung durch An/Aus-Schalten und
 * optional PWM-basiertes Dimming (0% -> 100% -> 0%).
 * 
 * @return true wenn Test erfolgreich
 */
bool diagnostics_test_backlight();

/**
 * @brief Test 3: Button-Eingabe Live-Test
 * 
 * Zeigt Live-Feedback wenn Buttons A und B gedrückt werden.
 * Testet auch Debouncing-Logik.
 * 
 * @return true wenn mindestens ein Button erkannt wurde
 */
bool diagnostics_test_buttons();

/**
 * @brief Test 4: WiFi Promiscuous Mode und Channel Hopping
 * 
 * Initialisiert WiFi im Promiscuous Mode, aktiviert Channel Hopping
 * und zählt empfangene Frames über mehrere Kanäle.
 * 
 * @return true wenn WiFi erfolgreich initialisiert und Frames empfangen
 */
bool diagnostics_test_wifi();

/**
 * @brief Test 5: BLE Scanning
 * 
 * Initialisiert NimBLE, startet einen Scan und zählt gefundene
 * BLE-Geräte. Zeigt Informationen zum letzten gefundenen Gerät an.
 * 
 * @return true wenn BLE erfolgreich initialisiert und mindestens ein Gerät gefunden
 */
bool diagnostics_test_ble();

/**
 * @brief Test 6: PSRAM und Heap Memory
 * 
 * Überprüft PSRAM-Verfügbarkeit, testet Allocation und zeigt
 * aktuellen Heap/PSRAM Status an.
 * 
 * @return true wenn PSRAM verfügbar und funktionsfähig
 */
bool diagnostics_test_memory();

/**
 * @brief Gibt Test-Ergebnis als JSON über Serial aus
 * 
 * Strukturierte JSON-Ausgabe für jeden Test mit:
 * - test_name: Name des Tests
 * - status: "PASS" oder "FAIL"
 * - details: Zusätzliche Informationen
 * - timestamp: Laufzeit in ms
 * 
 * @param test_name Name des Tests (z.B. "display", "wifi")
 * @param passed true für PASS, false für FAIL
 * @param details Optional: zusätzliche Details als String
 */
void diagnostics_output_result_json(const char* test_name, bool passed, const char* details = nullptr);

/**
 * @brief Zeigt Test-Status auf Display an
 * 
 * Rendert aktuellen Test-Status mit PASS/FAIL Indikator
 * und optionaler Detail-Information.
 * 
 * @param test_name Name des Tests
 * @param passed true für PASS (grün), false für FAIL (rot)
 * @param details Optional: zusätzliche Details
 */
void diagnostics_display_status(const char* test_name, bool passed, const char* details = nullptr);
