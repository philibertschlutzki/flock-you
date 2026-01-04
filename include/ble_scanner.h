/**
 * @file ble_scanner.h
 * @brief BLE Scanner für Flock Safety und Raven Detection
 * 
 * Dieses Modul implementiert BLE-Scanning mit Active Scan,
 * Erkennung von Flock Safety Geräten anhand von Device-Namen,
 * MAC-Präfixen und Raven Service UUIDs.
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Initialisiert den BLE Scanner
 * 
 * Initialisiert NimBLE, konfiguriert Scan-Parameter und
 * registriert Callbacks für Advertisement-Erkennung.
 */
void ble_scanner_init();

/**
 * @brief Verwaltet BLE Scan-Zyklus
 * 
 * Startet periodische BLE-Scans basierend auf konfiguriertem
 * BLE_SCAN_INTERVAL und BLE_SCAN_DURATION.
 */
void ble_scanner_tick();

/**
 * @brief Prüft ob BLE gerade scannt
 * @return true wenn Scan aktiv, sonst false
 */
bool ble_scanner_is_scanning();

/**
 * @brief Prüft ob BLE Device-Name einem Flock Safety Muster entspricht
 * @param name Der zu prüfende Device-Name
 * @return true wenn Name-Muster übereinstimmt, sonst false
 */
bool ble_check_device_name_pattern(const char* name);
