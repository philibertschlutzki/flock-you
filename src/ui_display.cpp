/**
 * @file ui_display.cpp
 * @brief Display UI Implementierung für LILYGO T-Display-S3
 */

#include "config.h"

#if FLOCKYOU_HAS_DISPLAY

#include "ui_display.h"
#include <TFT_eSPI.h>

// TFT Display Objekt
static TFT_eSPI tft = TFT_eSPI();

// UI State
static unsigned long last_ui_render = 0;
static unsigned long last_alert_until = 0;
static String last_alert_title;
static String last_alert_line;
static int last_alert_rssi = 0;

// Externe globale Variablen (aus main.cpp / wird später refaktoriert)
extern bool device_in_range;

void ui_display_init()
{
    tft.init();
    tft.setRotation(1);  // Landscape mode (320x170)
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    // Splash Screen
    tft.drawString("Flock You", 4, 4, 4);
    tft.drawString("LILYGO T-Display-S3", 4, 36, 2);
    tft.drawString("Initialisierung...", 4, 60, 2);

    // Backlight aktivieren
    pinMode(TFT_BACKLIGHT_PIN, OUTPUT);
    digitalWrite(TFT_BACKLIGHT_PIN, HIGH);

    delay(2000);  // Splash Screen für 2 Sekunden anzeigen
}

void ui_display_set_alert(const String& title, const String& line, int rssi)
{
    last_alert_title = title;
    last_alert_line = line;
    last_alert_rssi = rssi;
    last_alert_until = millis() + ALERT_DISPLAY_MS;
}

/**
 * @brief Rendert Alert-Screen
 */
static void display_render_alert()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("ALERT", 4, 4, 4);

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(last_alert_title, 4, 40, 2);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    
    // Wrap long lines
    if (last_alert_line.length() > 25) {
        String line1 = last_alert_line.substring(0, 25);
        String line2 = last_alert_line.substring(25);
        tft.drawString(line1, 4, 62, 2);
        if (line2.length() > 0) {
            tft.drawString(line2, 4, 80, 2);
        }
    } else {
        tft.drawString(last_alert_line, 4, 62, 2);
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "RSSI: %d dBm", last_alert_rssi);
    tft.drawString(buf, 4, 100, 2);

    // Signalstärke visualisieren
    int signal_bars = 0;
    if (last_alert_rssi > -50) signal_bars = 4;
    else if (last_alert_rssi > -60) signal_bars = 3;
    else if (last_alert_rssi > -70) signal_bars = 2;
    else if (last_alert_rssi > -80) signal_bars = 1;

    int bar_x = 4;
    int bar_y = 125;
    for (int i = 0; i < 4; i++) {
        uint16_t color = (i < signal_bars) ? TFT_GREEN : TFT_DARKGREY;
        tft.fillRect(bar_x + i * 12, bar_y + (3-i)*8, 8, 32 - (3-i)*8, color);
    }
}

/**
 * @brief Rendert Status-Screen
 * @param ble_scanning true wenn BLE gerade scannt
 */
static void display_render_status(bool ble_scanning)
{
    // Externe Funktion für aktuellen WiFi-Kanal
    extern uint8_t wifi_sniffer_get_current_channel();
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Flock You", 4, 4, 4);

    char buf[64];
    snprintf(buf, sizeof(buf), "WiFi CH: %u", wifi_sniffer_get_current_channel());
    tft.drawString(buf, 4, 36, 2);

    snprintf(buf, sizeof(buf), "BLE: %s", ble_scanning ? "scanning" : "idle");
    tft.drawString(buf, 4, 56, 2);

    snprintf(buf, sizeof(buf), "In range: %s", device_in_range ? "YES" : "NO");
    tft.setTextColor(device_in_range ? TFT_RED : TFT_GREEN, TFT_BLACK);
    tft.drawString(buf, 4, 76, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    float up = millis() / 1000.0f;
    snprintf(buf, sizeof(buf), "Uptime: %.1fs", up);
    tft.drawString(buf, 4, 96, 2);

    // Status-Indikator
    int indicator_x = 300;
    int indicator_y = 10;
    uint16_t indicator_color = ble_scanning ? TFT_BLUE : TFT_DARKGREY;
    tft.fillCircle(indicator_x, indicator_y, 8, indicator_color);
}

void ui_display_tick(bool ble_scanning)
{
    unsigned long now = millis();
    if (now - last_ui_render < UI_RENDER_INTERVAL_MS) return;
    last_ui_render = now;

    if (last_alert_until > now) {
        display_render_alert();
    } else {
        display_render_status(ble_scanning);
    }
}

void ui_display_set_backlight(bool on)
{
    digitalWrite(TFT_BACKLIGHT_PIN, on ? HIGH : LOW);
}

void ui_display_set_brightness(uint8_t brightness)
{
    // PWM für Helligkeitssteuerung
    // Frequency: 5000 Hz, Resolution: 8 bit
    ledcSetup(0, 5000, 8);  // PWM Channel 0
    ledcAttachPin(TFT_BACKLIGHT_PIN, 0);
    ledcWrite(0, brightness);
}

#endif // FLOCKYOU_HAS_DISPLAY
