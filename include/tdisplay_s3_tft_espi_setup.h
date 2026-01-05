/**
 * @file tdisplay_s3_tft_espi_setup.h
 * @brief TFT_eSPI Konfiguration für LILYGO T-Display-S3
 * 
 * Diese Datei wird automatisch via PlatformIO Build-Flag eingebunden:
 *   -DUSER_SETUP_LOADED -include include/tdisplay_s3_tft_espi_setup.h
 * 
 * Zielhardware: LILYGO T-Display-S3 (ESP32-S3 + 1.9" ST7789 TFT)
 * 
 * WICHTIG: Dieses Board verwendet i8080 8-bit parallel interface, NICHT SPI!
 * Basiert auf TFT_eSPI Setup206_LilyGo_T_Display_S3.h
 * 
 * @see https://github.com/Bodmer/TFT_eSPI/blob/master/User_Setups/Setup206_LilyGo_T_Display_S3.h
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
// i8080 8-BIT PARALLEL INTERFACE CONFIGURATION
// ============================================================================

/**
 * @brief Enable i8080 8-bit parallel mode
 * 
 * LILYGO T-Display-S3 verwendet 8-bit parallel interface für höhere
 * Geschwindigkeit statt SPI.
 */
#define TFT_PARALLEL_8_BIT

/**
 * @brief Chip Select - Aktiviert Display für i8080-Kommunikation
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

/**
 * @brief Write Strobe - i8080 Write Clock Signal
 */
#define TFT_WR    8

/**
 * @brief Read Strobe - i8080 Read Clock Signal
 */
#define TFT_RD    9

/**
 * @brief 8-bit parallel data bus pins (D0-D7)
 * 
 * Definiert die GPIO-Pins für die 8 Datenleitungen des i8080 Interface.
 */
#define TFT_D0   39
#define TFT_D1   40
#define TFT_D2   41
#define TFT_D3   42
#define TFT_D4   45
#define TFT_D5   46
#define TFT_D6   47
#define TFT_D7   48

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

/**
 * @brief Alias für Application-Code Kompatibilität
 * 
 * Viele Application-Codes erwarten TFT_BACKLIGHT_PIN statt TFT_BL.
 * Diese Definitionen ermöglichen portable Code zwischen verschiedenen
 * TFT_eSPI Setups.
 */
#ifndef TFT_BACKLIGHT_PIN
#define TFT_BACKLIGHT_PIN TFT_BL
#endif

// ============================================================================
// BUS SPEED CONFIGURATION
// ============================================================================

/**
 * @brief i8080 Bus-Frequenz in Hz
 * 
 * 40 MHz ist ein guter Kompromiss zwischen Geschwindigkeit und Stabilität.
 * Bei Problemen auf 27000000 (27 MHz) reduzieren.
 */
#define TFT_WR_FREQ  40000000

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

