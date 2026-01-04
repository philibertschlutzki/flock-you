/**
 * @file ui_display.h
 * @brief Display UI für LILYGO T-Display-S3
 * 
 * Dieses Modul implementiert die grafische Benutzeroberfläche
 * auf dem ST7789 Display des LILYGO T-Display-S3.
 * 
 * Features:
 * - Status-Screen: WiFi Channel, BLE Status, Uptime, In-Range Indicator
 * - Alert-Screen: Letzte Detektion mit RSSI und Details
 * - Backlight-Steuerung
 */

#pragma once

#include <Arduino.h>

#if FLOCKYOU_HAS_DISPLAY

/**
 * @brief Initialisiert das Display
 * 
 * Initialisiert TFT_eSPI, setzt Rotation, aktiviert Backlight
 * und zeigt Splash-Screen.
 */
void ui_display_init();

/**
 * @brief Setzt Alert für Display-Anzeige
 * 
 * @param title Alert-Titel (z.B. "WIFI", "BLE", "RAVEN")
 * @param line Detail-Zeile (z.B. Gerätename, SSID)
 * @param rssi Signalstärke in dBm
 */
void ui_display_set_alert(const String& title, const String& line, int rssi);

/**
 * @brief Update-Funktion für Display (regelmäßig aufrufen)
 * 
 * Rendert entweder Alert-Screen oder Status-Screen
 * basierend auf aktuellem Zustand.
 * 
 * @param ble_scanning true wenn BLE gerade scannt
 */
void ui_display_tick(bool ble_scanning);

/**
 * @brief Setzt Backlight-Status
 * 
 * @param on true für Backlight an, false für aus
 */
void ui_display_set_backlight(bool on);

/**
 * @brief Setzt Backlight-Helligkeit (0-255)
 * 
 * @param brightness Helligkeitswert 0-255
 */
void ui_display_set_brightness(uint8_t brightness);

#endif // FLOCKYOU_HAS_DISPLAY
