/**
 * @file config.h
 * @brief Zentrale Konfigurationsdatei für Flock You Detection System
 * 
 * Diese Datei enthält alle Hardware-Pins, Compile-Time-Flags und 
 * grundlegende Konfigurationsparameter für das Flock You System.
 * 
 * Zielhardware: LILYGO T-Display-S3 (ESP32-S3 + ST7789 1.9")
 */

#pragma once

// ============================================================================
// COMPILE-TIME FEATURE FLAGS
// ============================================================================

/**
 * @def FLOCKYOU_NO_BUZZER
 * @brief Deaktiviert Audio-Funktionen (Buzzer)
 * 
 * Wenn auf 1 gesetzt, werden alle Buzzer-Funktionen deaktiviert.
 * Standard für LILYGO T-Display-S3 (kein Buzzer vorhanden).
 */
#ifndef FLOCKYOU_NO_BUZZER
#define FLOCKYOU_NO_BUZZER 0
#endif

/**
 * @def FLOCKYOU_HAS_DISPLAY
 * @brief Aktiviert Display-Funktionen (TFT)
 * 
 * Wenn auf 1 gesetzt, werden Display-UI-Funktionen aktiviert.
 * Standard für LILYGO T-Display-S3 (ST7789 Display vorhanden).
 */
#ifndef FLOCKYOU_HAS_DISPLAY
#define FLOCKYOU_HAS_DISPLAY 0
#endif

// ============================================================================
// HARDWARE PIN MAPPING - LILYGO T-Display-S3
// ============================================================================

/**
 * @brief Backlight-Pin für ST7789 Display
 * 
 * Der Backlight kann per PWM gesteuert werden für Helligkeitsanpassung.
 * GPIO 38 ist der Standard-Pin für LILYGO T-Display-S3.
 */
#define TFT_BACKLIGHT_PIN 38

/**
 * @brief Button A Pin (falls vorhanden auf T-Display-S3)
 * 
 * Verwendung: Toggle Status/Details Ansicht
 * Typisch GPIO 0 auf T-Display-S3 (Board-abhängig)
 */
#define BUTTON_A_PIN 0

/**
 * @brief Button B Pin (falls vorhanden auf T-Display-S3)
 * 
 * Verwendung: Toggle Diagnostics/Normal Mode
 * Typisch GPIO 14 auf T-Display-S3 (Board-abhängig)
 */
#define BUTTON_B_PIN 14

// ============================================================================
// WIFI PROMISCUOUS MODE CONFIGURATION
// ============================================================================

#define MAX_CHANNEL 13                  ///< Maximaler WiFi-Kanal
#define CHANNEL_HOP_INTERVAL 500        ///< Channel-Wechsel-Intervall in ms

// ============================================================================
// BLE SCANNING CONFIGURATION
// ============================================================================

#define BLE_SCAN_DURATION 1      ///< BLE Scan-Dauer in Sekunden
#define BLE_SCAN_INTERVAL 5000   ///< Millisekunden zwischen BLE-Scans

// ============================================================================
// DISPLAY UI CONFIGURATION
// ============================================================================

#define UI_RENDER_INTERVAL_MS 200    ///< Display-Update-Intervall in ms
#define ALERT_DISPLAY_MS 8000        ///< Dauer der Alert-Anzeige in ms

// ============================================================================
// DETECTION PATTERN LIMITS
// ============================================================================

#define MAX_SSID_PATTERNS 10      ///< Maximale Anzahl SSID-Muster
#define MAX_MAC_PATTERNS 50       ///< Maximale Anzahl MAC-Präfix-Muster
#define MAX_DEVICE_NAMES 20       ///< Maximale Anzahl BLE-Device-Name-Muster

// ============================================================================
// DIAGNOSTICS MODE CONFIGURATION
// ============================================================================

#define DIAGNOSTICS_BOOT_HOLD_MS 3000  ///< Button-Hold-Zeit für Diagnostics beim Boot (ms)
