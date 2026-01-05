/**
 * @file ble_scanner.cpp
 * @brief BLE Scanner Implementierung
 */

#include "ble_scanner.h"
#include "config.h"
#include "blacklist.h"
#include "output.h"
#include "state.h"
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <ArduinoJson.h>
#include <string.h>

// Globale Variablen für BLE Scanner
static NimBLEScan* pBLEScan = nullptr;
static unsigned long last_ble_scan = 0;

/**
 * @brief BLE Advertisement Callback-Handler
 * 
 * Wird für jedes empfangene BLE Advertisement aufgerufen.
 * Prüft gegen Blacklist - nur unbekannte Geräte werden gemeldet.
 */
class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override {

        NimBLEAddress addr = advertisedDevice->getAddress();
        std::string addrStr = addr.toString();
        uint8_t mac[6];
        sscanf(addrStr.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

        int rssi = advertisedDevice->getRSSI();
        std::string name = "";
        if (advertisedDevice->haveName()) {
            name = advertisedDevice->getName();
        }

        // Check blacklist - filter out known devices
        if (is_known_ble(mac)) {
            // Device is known - skip (no output)
            return;
        }
        
        // Device is unknown - report it
        // Name is included for info but not used for filtering
        output_ble_detection_json(addrStr.c_str(), name.c_str(), rssi, "unknown");
        
        if (!triggered) {
            output_trigger_detection();
            triggered = true;
        }
        last_detection_time = millis();
    }
};

void ble_scanner_init()
{
    Serial.println("Initializing BLE scanner...");
    NimBLEDevice::init("");
    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    Serial.println("BLE scanner initialized (unknown-only mode)");
}

void ble_scanner_tick()
{
    if (!pBLEScan) return;

    if (millis() - last_ble_scan >= BLE_SCAN_INTERVAL && !pBLEScan->isScanning()) {
        Serial.println("[BLE] scan...");
        pBLEScan->start(BLE_SCAN_DURATION, false);
        last_ble_scan = millis();
    }

    if (!pBLEScan->isScanning() && millis() - last_ble_scan > BLE_SCAN_DURATION * 1000) {
        pBLEScan->clearResults();
    }
}

bool ble_scanner_is_scanning()
{
    if (!pBLEScan) return false;
    return pBLEScan->isScanning();
}
