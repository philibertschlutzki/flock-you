/**
 * @file diagnostics.cpp
 * @brief Implementierung des Hardware-Selbsttest-Systems
 * 
 * @author Flock You Team
 * @date 2026-01-04
 */

#include "diagnostics.h"
#include "config.h"
#include <Arduino.h>
#include <ArduinoJson.h>

// WiFi und BLE für Tests
#include <WiFi.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>

#if FLOCKYOU_HAS_DISPLAY
#include <TFT_eSPI.h>
extern TFT_eSPI tft;  // Defined in main.cpp
#endif

// ============================================================================
// GLOBALE DIAGNOSTICS-VARIABLEN
// ============================================================================

static volatile uint32_t wifi_frame_count = 0;
static volatile int last_wifi_rssi = 0;
static uint32_t ble_device_count = 0;
static String last_ble_device_mac = "";
static String last_ble_device_name = "";
static int last_ble_rssi = 0;

// ============================================================================
// HILFSFUNKTIONEN
// ============================================================================

/**
 * @brief WiFi Promiscuous Callback für Frame-Zählung
 */
static void IRAM_ATTR diagnostics_wifi_sniffer_callback(void* buff, wifi_promiscuous_pkt_type_t type)
{
    (void)type;
    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
    wifi_frame_count++;
    last_wifi_rssi = ppkt->rx_ctrl.rssi;
}

/**
 * @brief BLE Scan Callback für Geräte-Zählung
 */
class DiagnosticsBLECallback: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override {
        ble_device_count++;
        last_ble_device_mac = advertisedDevice->getAddress().toString().c_str();
        last_ble_rssi = advertisedDevice->getRSSI();
        if (advertisedDevice->haveName()) {
            last_ble_device_name = advertisedDevice->getName().c_str();
        } else {
            last_ble_device_name = "";
        }
    }
};

// ============================================================================
// DIAGNOSTICS ENTRY CHECK
// ============================================================================

bool diagnostics_should_enter()
{
#if FLOCKYOU_HAS_DISPLAY
    // Button A beim Boot prüfen
    pinMode(BUTTON_A_PIN, INPUT_PULLUP);
    
    // Warte kurz für stabile Werte
    delay(50);
    
    // Prüfe ob Button A gedrückt ist (LOW bei PULLUP)
    if (digitalRead(BUTTON_A_PIN) == LOW) {
        Serial.println("[DIAGNOSTICS] Button A detected - checking hold time...");
        
        // Warte bis Button losgelassen oder Timeout
        unsigned long start = millis();
        while (digitalRead(BUTTON_A_PIN) == LOW) {
            if (millis() - start > DIAGNOSTICS_BOOT_HOLD_MS) {
                Serial.println("[DIAGNOSTICS] Button A held long enough - entering diagnostics mode");
                return true;
            }
            delay(10);
        }
        
        Serial.println("[DIAGNOSTICS] Button A released too early - normal boot");
        return false;
    }
#endif
    
    return false;
}

// ============================================================================
// JSON OUTPUT
// ============================================================================

void diagnostics_output_result_json(const char* test_name, bool passed, const char* details)
{
    StaticJsonDocument<512> doc;
    
    doc["type"] = "diagnostics_result";
    doc["test_name"] = test_name;
    doc["status"] = passed ? "PASS" : "FAIL";
    doc["timestamp_ms"] = millis();
    doc["uptime_sec"] = millis() / 1000.0;
    
    if (details != nullptr && strlen(details) > 0) {
        doc["details"] = details;
    }
    
    serializeJson(doc, Serial);
    Serial.println();
}

// ============================================================================
// DISPLAY STATUS
// ============================================================================

void diagnostics_display_status(const char* test_name, bool passed, const char* details)
{
#if FLOCKYOU_HAS_DISPLAY
    // Hintergrundfarbe basierend auf Status
    uint16_t bg_color = passed ? TFT_DARKGREEN : TFT_MAROON;
    uint16_t text_color = TFT_WHITE;
    
    tft.fillScreen(TFT_BLACK);
    
    // Header
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("DIAGNOSTICS", 4, 4, 4);
    
    // Test Name
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    char test_title[64];
    snprintf(test_title, sizeof(test_title), "Test: %s", test_name);
    tft.drawString(test_title, 4, 36, 2);
    
    // Status Box
    int box_y = 60;
    int box_h = 30;
    tft.fillRect(4, box_y, 312, box_h, bg_color);
    tft.setTextColor(text_color, bg_color);
    const char* status_text = passed ? "PASS" : "FAIL";
    tft.drawString(status_text, 10, box_y + 6, 4);
    
    // Details
    if (details != nullptr && strlen(details) > 0) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString(details, 4, 100, 2);
    }
#else
    (void)test_name;
    (void)passed;
    (void)details;
#endif
}

// ============================================================================
// TEST 1: DISPLAY
// ============================================================================

bool diagnostics_test_display()
{
#if FLOCKYOU_HAS_DISPLAY
    Serial.println("[DIAGNOSTICS] Testing display...");
    
    // Test 1: Schwarzer Screen
    tft.fillScreen(TFT_BLACK);
    delay(500);
    
    // Test 2: Rot
    tft.fillScreen(TFT_RED);
    delay(500);
    
    // Test 3: Grün
    tft.fillScreen(TFT_GREEN);
    delay(500);
    
    // Test 4: Blau
    tft.fillScreen(TFT_BLUE);
    delay(500);
    
    // Test 5: Weiß
    tft.fillScreen(TFT_WHITE);
    delay(500);
    
    // Test 6: Text-Rendering
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Display Test", 10, 10, 4);
    tft.drawString("Font Test 123", 10, 40, 2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Colors: OK", 10, 60, 2);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Text: OK", 10, 80, 2);
    
    delay(1500);
    
    diagnostics_output_result_json("display", true, "RGB + Text OK");
    return true;
#else
    diagnostics_output_result_json("display", false, "No display available");
    return false;
#endif
}

// ============================================================================
// TEST 2: BACKLIGHT
// ============================================================================

bool diagnostics_test_backlight()
{
#if FLOCKYOU_HAS_DISPLAY
    Serial.println("[DIAGNOSTICS] Testing backlight...");
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Backlight Test", 4, 4, 4);
    
    // Setup Backlight Pin
    pinMode(TFT_BACKLIGHT_PIN, OUTPUT);
    
    // Test 1: Off
    tft.drawString("Backlight: OFF", 4, 40, 2);
    digitalWrite(TFT_BACKLIGHT_PIN, LOW);
    delay(1000);
    
    // Test 2: On
    digitalWrite(TFT_BACKLIGHT_PIN, HIGH);
    tft.drawString("Backlight: ON ", 4, 40, 2);
    delay(1000);
    
    // Test 3: PWM Ramping
    tft.drawString("PWM Ramp Test", 4, 60, 2);
    
    // Ramp up
    for (int i = 0; i <= 255; i += 5) {
        analogWrite(TFT_BACKLIGHT_PIN, i);
        delay(20);
    }
    
    // Ramp down
    for (int i = 255; i >= 0; i -= 5) {
        analogWrite(TFT_BACKLIGHT_PIN, i);
        delay(20);
    }
    
    // Back to full brightness
    analogWrite(TFT_BACKLIGHT_PIN, 255);
    
    diagnostics_output_result_json("backlight", true, "On/Off + PWM OK");
    return true;
#else
    diagnostics_output_result_json("backlight", false, "No display available");
    return false;
#endif
}

// ============================================================================
// TEST 3: BUTTONS
// ============================================================================

bool diagnostics_test_buttons()
{
#if FLOCKYOU_HAS_DISPLAY
    Serial.println("[DIAGNOSTICS] Testing buttons...");
    
    pinMode(BUTTON_A_PIN, INPUT_PULLUP);
    pinMode(BUTTON_B_PIN, INPUT_PULLUP);
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("Button Test", 4, 4, 4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Press buttons A & B", 4, 36, 2);
    tft.drawString("(10 sec timeout)", 4, 56, 2);
    
    unsigned long start = millis();
    bool button_a_tested = false;
    bool button_b_tested = false;
    
    while (millis() - start < 10000) {
        bool a_pressed = (digitalRead(BUTTON_A_PIN) == LOW);
        bool b_pressed = (digitalRead(BUTTON_B_PIN) == LOW);
        
        if (a_pressed) button_a_tested = true;
        if (b_pressed) button_b_tested = true;
        
        // Live-Anzeige
        tft.fillRect(4, 90, 312, 60, TFT_BLACK);
        tft.setTextColor(a_pressed ? TFT_GREEN : TFT_RED, TFT_BLACK);
        tft.drawString(a_pressed ? "Button A: PRESSED " : "Button A: Released", 4, 90, 2);
        
        tft.setTextColor(b_pressed ? TFT_GREEN : TFT_RED, TFT_BLACK);
        tft.drawString(b_pressed ? "Button B: PRESSED " : "Button B: Released", 4, 110, 2);
        
        delay(50);  // Debounce
    }
    
    bool passed = button_a_tested && button_b_tested;
    char details[64];
    snprintf(details, sizeof(details), "A:%s B:%s", 
             button_a_tested ? "OK" : "FAIL",
             button_b_tested ? "OK" : "FAIL");
    
    diagnostics_output_result_json("buttons", passed, details);
    return passed;
#else
    diagnostics_output_result_json("buttons", false, "No buttons available");
    return false;
#endif
}

// ============================================================================
// TEST 4: WIFI
// ============================================================================

bool diagnostics_test_wifi()
{
    Serial.println("[DIAGNOSTICS] Testing WiFi...");
    
#if FLOCKYOU_HAS_DISPLAY
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("WiFi Test", 4, 4, 4);
#endif
    
    // WiFi init
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    // Promiscuous mode
    wifi_frame_count = 0;
    last_wifi_rssi = 0;
    
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&diagnostics_wifi_sniffer_callback);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    
    Serial.println("[DIAGNOSTICS] WiFi promiscuous mode enabled");
    
    // Channel hopping test
    for (int ch = 1; ch <= 13; ch++) {
        esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
        
#if FLOCKYOU_HAS_DISPLAY
        tft.fillRect(4, 36, 312, 100, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        char buf[64];
        snprintf(buf, sizeof(buf), "Channel: %d", ch);
        tft.drawString(buf, 4, 36, 2);
        snprintf(buf, sizeof(buf), "Frames: %lu", wifi_frame_count);
        tft.drawString(buf, 4, 56, 2);
        if (last_wifi_rssi != 0) {
            snprintf(buf, sizeof(buf), "Last RSSI: %d dBm", last_wifi_rssi);
            tft.drawString(buf, 4, 76, 2);
        }
#endif
        
        delay(200);
    }
    
    esp_wifi_set_promiscuous(false);
    
    bool passed = (wifi_frame_count > 0);
    char details[64];
    snprintf(details, sizeof(details), "Frames: %lu, RSSI: %d dBm", wifi_frame_count, last_wifi_rssi);
    
    diagnostics_output_result_json("wifi", passed, details);
    return passed;
}

// ============================================================================
// TEST 5: BLE
// ============================================================================

bool diagnostics_test_ble()
{
    Serial.println("[DIAGNOSTICS] Testing BLE...");
    
#if FLOCKYOU_HAS_DISPLAY
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("BLE Test", 4, 4, 4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Scanning...", 4, 36, 2);
#endif
    
    // BLE init
    ble_device_count = 0;
    last_ble_device_mac = "";
    last_ble_device_name = "";
    last_ble_rssi = 0;
    
    NimBLEDevice::init("");
    NimBLEScan* pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new DiagnosticsBLECallback());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    
    // Scan für 5 Sekunden
    Serial.println("[DIAGNOSTICS] BLE scanning for 5 seconds...");
    pBLEScan->start(5, false);
    
    // Warte auf Scan-Ende
    while (pBLEScan->isScanning()) {
#if FLOCKYOU_HAS_DISPLAY
        tft.fillRect(4, 56, 312, 80, TFT_BLACK);
        char buf[64];
        snprintf(buf, sizeof(buf), "Devices: %lu", ble_device_count);
        tft.drawString(buf, 4, 56, 2);
        
        if (last_ble_device_mac.length() > 0) {
            tft.drawString("Last device:", 4, 76, 2);
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.drawString(last_ble_device_mac.c_str(), 4, 96, 2);
            if (last_ble_device_name.length() > 0) {
                tft.drawString(last_ble_device_name.c_str(), 4, 116, 2);
            }
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            snprintf(buf, sizeof(buf), "RSSI: %d dBm", last_ble_rssi);
            tft.drawString(buf, 4, 136, 2);
        }
#endif
        delay(100);
    }
    
    pBLEScan->clearResults();
    NimBLEDevice::deinit(true);
    
    bool passed = (ble_device_count > 0);
    char details[128];
    if (passed) {
        snprintf(details, sizeof(details), "Found %lu devices, Last: %s", 
                 ble_device_count, last_ble_device_mac.c_str());
    } else {
        snprintf(details, sizeof(details), "No devices found");
    }
    
    diagnostics_output_result_json("ble", passed, details);
    return passed;
}

// ============================================================================
// TEST 6: MEMORY (PSRAM + HEAP)
// ============================================================================

bool diagnostics_test_memory()
{
    Serial.println("[DIAGNOSTICS] Testing memory...");
    
#if FLOCKYOU_HAS_DISPLAY
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("Memory Test", 4, 4, 4);
#endif
    
    // Heap Info
    uint32_t free_heap = ESP.getFreeHeap();
    uint32_t total_heap = ESP.getHeapSize();
    uint32_t min_free_heap = ESP.getMinFreeHeap();
    
    // PSRAM Info
    uint32_t free_psram = ESP.getFreePsram();
    uint32_t total_psram = ESP.getPsramSize();
    
    bool has_psram = (total_psram > 0);
    
#if FLOCKYOU_HAS_DISPLAY
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    char buf[64];
    
    snprintf(buf, sizeof(buf), "Heap: %lu / %lu KB", free_heap / 1024, total_heap / 1024);
    tft.drawString(buf, 4, 36, 2);
    
    snprintf(buf, sizeof(buf), "Min Free: %lu KB", min_free_heap / 1024);
    tft.drawString(buf, 4, 56, 2);
    
    if (has_psram) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("PSRAM: Available", 4, 80, 2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        snprintf(buf, sizeof(buf), "PSRAM: %lu / %lu KB", free_psram / 1024, total_psram / 1024);
        tft.drawString(buf, 4, 100, 2);
    } else {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("PSRAM: Not available", 4, 80, 2);
    }
#endif
    
    // PSRAM Test Allocation (falls verfügbar)
    bool psram_test_passed = true;
    if (has_psram) {
        void* test_alloc = heap_caps_malloc(1024 * 100, MALLOC_CAP_SPIRAM);  // 100 KB from PSRAM
        if (test_alloc != nullptr) {
            heap_caps_free(test_alloc);
            Serial.println("[DIAGNOSTICS] PSRAM allocation test: OK");
        } else {
            psram_test_passed = false;
            Serial.println("[DIAGNOSTICS] PSRAM allocation test: FAILED");
        }
    }
    
    char details[128];
    snprintf(details, sizeof(details), "Heap:%luKB PSRAM:%s(%luKB)", 
             free_heap / 1024, 
             has_psram ? "OK" : "N/A",
             has_psram ? (free_psram / 1024) : 0);
    
    diagnostics_output_result_json("memory", psram_test_passed, details);
    
    delay(2000);
    return psram_test_passed;
}

// ============================================================================
// MAIN DIAGNOSTICS RUNNER
// ============================================================================

void diagnostics_run_all_tests()
{
    Serial.println("\n");
    Serial.println("========================================");
    Serial.println("FLOCK YOU HARDWARE DIAGNOSTICS");
    Serial.println("LILYGO T-Display-S3");
    Serial.println("========================================");
    Serial.println();
    
#if FLOCKYOU_HAS_DISPLAY
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("DIAGNOSTICS", 60, 60, 4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Starting tests...", 80, 100, 2);
    delay(2000);
#endif
    
    // Führe alle Tests durch
    bool test_results[6];
    
    test_results[0] = diagnostics_test_display();
    diagnostics_display_status("Display", test_results[0], "RGB + Text");
    delay(2000);
    
    test_results[1] = diagnostics_test_backlight();
    diagnostics_display_status("Backlight", test_results[1], "PWM Control");
    delay(2000);
    
    test_results[2] = diagnostics_test_buttons();
    diagnostics_display_status("Buttons", test_results[2], "A + B Input");
    delay(2000);
    
    test_results[3] = diagnostics_test_wifi();
    diagnostics_display_status("WiFi", test_results[3], "Promiscuous Mode");
    delay(2000);
    
    test_results[4] = diagnostics_test_ble();
    diagnostics_display_status("BLE", test_results[4], "Device Scan");
    delay(2000);
    
    test_results[5] = diagnostics_test_memory();
    diagnostics_display_status("Memory", test_results[5], "PSRAM + Heap");
    delay(2000);
    
    // Zusammenfassung
    int passed_count = 0;
    int total_count = 6;
    for (int i = 0; i < total_count; i++) {
        if (test_results[i]) passed_count++;
    }
    
    Serial.println("\n========================================");
    Serial.printf("DIAGNOSTICS COMPLETE: %d/%d PASSED\n", passed_count, total_count);
    Serial.println("========================================\n");
    
#if FLOCKYOU_HAS_DISPLAY
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("SUMMARY", 4, 4, 4);
    
    uint16_t summary_color = (passed_count == total_count) ? TFT_GREEN : 
                             (passed_count > total_count / 2) ? TFT_YELLOW : TFT_RED;
    
    tft.setTextColor(summary_color, TFT_BLACK);
    char summary[64];
    snprintf(summary, sizeof(summary), "%d / %d PASSED", passed_count, total_count);
    tft.drawString(summary, 4, 40, 4);
    
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Press RESET to reboot", 4, 140, 2);
#endif
    
    // Endlosschleife (nur Reset zum Verlassen)
    while (true) {
        delay(1000);
    }
}
