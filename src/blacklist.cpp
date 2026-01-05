/**
 * @file blacklist.cpp
 * @brief Combined blacklist checking implementation
 */

#include "blacklist.h"
#include "known_blacklist_generated.h"
#include "session_filter.h"
#include <cstring>

/**
 * @brief Check if SSID matches compile-time blacklist
 */
static bool ssid_in_compile_time_blacklist(const char* ssid)
{
    if (!ssid || !*ssid) return false;
    
    for (size_t i = 0; i < known_wifi_ssids_count; i++) {
        // Case-sensitive exact match
        if (strcmp(ssid, known_wifi_ssids[i]) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Check if MAC matches WiFi compile-time blacklist
 */
static bool mac_in_wifi_compile_time_blacklist(const uint8_t mac[6])
{
    if (!mac) return false;
    
    for (size_t i = 0; i < known_wifi_macs_count; i++) {
        if (memcmp(mac, known_wifi_macs[i], 6) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Check if MAC matches BLE compile-time blacklist
 */
static bool mac_in_ble_compile_time_blacklist(const uint8_t mac[6])
{
    if (!mac) return false;
    
    for (size_t i = 0; i < known_ble_macs_count; i++) {
        if (memcmp(mac, known_ble_macs[i], 6) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Check if OUI (first 3 bytes) matches compile-time blacklist
 */
static bool oui_in_compile_time_blacklist(const uint8_t mac[6])
{
    if (!mac) return false;
    
    for (size_t i = 0; i < known_ouis_count; i++) {
        if (memcmp(mac, known_ouis[i], 3) == 0) {
            return true;
        }
    }
    return false;
}

bool is_known_wifi(const char* ssid_nullable, 
                   const uint8_t mac_a_nullable[6], 
                   const uint8_t mac_b_nullable[6])
{
    // Check SSID against compile-time blacklist
    if (ssid_nullable && *ssid_nullable) {
        if (ssid_in_compile_time_blacklist(ssid_nullable)) {
            return true;
        }
    }
    
    // Check MAC addresses against compile-time blacklists
    if (mac_a_nullable) {
        if (mac_in_wifi_compile_time_blacklist(mac_a_nullable)) {
            return true;
        }
        if (oui_in_compile_time_blacklist(mac_a_nullable)) {
            return true;
        }
    }
    
    if (mac_b_nullable) {
        if (mac_in_wifi_compile_time_blacklist(mac_b_nullable)) {
            return true;
        }
        if (oui_in_compile_time_blacklist(mac_b_nullable)) {
            return true;
        }
    }
    
    // Check session filter
    if (session_filter_is_known_wifi(ssid_nullable, mac_a_nullable, mac_b_nullable)) {
        return true;
    }
    
    return false;
}

bool is_known_ble(const uint8_t mac[6])
{
    if (!mac) return false;
    
    // Check MAC against compile-time blacklist
    if (mac_in_ble_compile_time_blacklist(mac)) {
        return true;
    }
    
    // Check OUI against compile-time blacklist
    if (oui_in_compile_time_blacklist(mac)) {
        return true;
    }
    
    // Check session filter
    if (session_filter_is_known_ble(mac)) {
        return true;
    }
    
    return false;
}
