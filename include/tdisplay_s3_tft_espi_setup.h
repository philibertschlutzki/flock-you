#pragma once

// TFT_eSPI setup injected via PlatformIO build flag:
//   -DUSER_SETUP_LOADED -include include/tdisplay_s3_tft_espi_setup.h
//
// Target: LILYGO T-Display-S3 (ESP32-S3 + 1.9" ST7789 TFT)
// NOTE: Board revisions exist; if your panel does not work, adjust pins.

#define ST7789_DRIVER

// Native panel size is 170x320 (portrait). Firmware uses rotation=1 (landscape).
#define TFT_WIDTH  170
#define TFT_HEIGHT 320

// Common T-Display-S3 ST7789 SPI pinout (seen in many example projects)
#define TFT_MOSI 17
#define TFT_SCLK 18
#define TFT_CS    6
#define TFT_DC    7
#define TFT_RST   5

// Backlight control
#define TFT_BL   38
#define TFT_BACKLIGHT_ON HIGH

// SPI speed
#define SPI_FREQUENCY  40000000

// Fonts
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
