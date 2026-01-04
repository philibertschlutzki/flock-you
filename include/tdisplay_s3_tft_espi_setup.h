/**
 * @file tdisplay_s3_tft_espi_setup.h
 * @brief TFT_eSPI Konfiguration für LILYGO T-Display-S3
 * 
 * Diese Datei wird automatisch via PlatformIO Build-Flag eingebunden:
 *   -DUSER_SETUP_LOADED -include include/tdisplay_s3_tft_espi_setup.h
 * 
 * Zielhardware: LILYGO T-Display-S3 (ESP32-S3 + 1.9" ST7789 TFT)
 * 
 * WICHTIG: Es gibt verschiedene Board-Revisionen!
 * Falls das Display nicht funktioniert, überprüfe die Pin-Belegung
 * und passe die Werte entsprechend an.
 * 
 * Typische Probleme:
 * - Display bleibt schwarz -> Backlight-Pin prüfen (GPIO 38)
 * - Falsche Farben -> Rotation oder RGB/BGR Order prüfen
 * - Kein Signal -> SPI Pins (MOSI, SCLK, CS, DC, RST) verifizieren
 */

#pragma once

// ============================================================================
// DISPLAY DRIVER CONFIGURATION
// ============================================================================

/**
 * @brief ST7789 Display-Treiber für T-Display-S3
 */
#define ST7789_DRIVER

/**
 * @brief Native Panel-Größe: 170x320 Pixel (Portrait)
 * 
 * Die Firmware verwendet rotation=1 für Landscape-Modus (320x170).
 */
#define TFT_WIDTH  170
#define TFT_HEIGHT 320

// ============================================================================
// SPI PIN CONFIGURATION - LILYGO T-Display-S3 Standard Pinout
// ============================================================================

/**
 * @brief SPI MOSI (Master Out Slave In) - Daten zum Display
 */
#define TFT_MOSI 17

/**
 * @brief SPI Clock - Takt-Signal für Display
 */
#define TFT_SCLK 18

/**
 * @brief Chip Select - Aktiviert Display für SPI-Kommunikation
 */
#define TFT_CS    6

/**
 * @brief Data/Command - Unterscheidet zwischen Daten und Befehlen
 */
#define TFT_DC    7

/**
 * @brief Reset - Hardware-Reset für Display
 */
#define TFT_RST   5

// ============================================================================
// BACKLIGHT CONFIGURATION
// ============================================================================

/**
 * @brief Backlight Control Pin
 * 
 * GPIO 38 steuert die Display-Hintergrundbeleuchtung.
 * Kann per PWM für Helligkeitssteuerung verwendet werden.
 */
#define TFT_BL   38

/**
 * @brief Backlight-Status für "An"
 * 
 * HIGH = Backlight an, LOW = Backlight aus
 */
#define TFT_BACKLIGHT_ON HIGH

// ============================================================================
// SPI SPEED CONFIGURATION
// ============================================================================

/**
 * @brief SPI Bus-Frequenz in Hz
 * 
 * 40 MHz ist ein guter Kompromiss zwischen Geschwindigkeit und Stabilität.
 * Bei Problemen auf 27000000 (27 MHz) reduzieren.
 */
#define SPI_FREQUENCY  40000000

// ============================================================================
// FONT CONFIGURATION
// ============================================================================

/**
 * @brief Aktivierte TFT_eSPI Fonts
 * 
 * GLCD = Standard Small Font
 * FONT2 = Medium Font
 * FONT4 = Large Font
 */
#define LOAD_GLCD   ///< Original Adafruit small font
#define LOAD_FONT2  ///< Small 16 pixel high font
#define LOAD_FONT4  ///< Medium 26 pixel high font

