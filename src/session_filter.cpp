/**
 * @file session_filter.cpp
 * @brief RAM-only session filter implementation
 */

#include "session_filter.h"
#include <vector>
#include <string>
#include <cstring>

// In-memory session filter storage
static std::vector<std::string> session_ssids;
static std::vector<std::array<uint8_t, 6>> session_wifi_macs;
static std::vector<std::array<uint8_t, 6>> session_ble_macs;
static std::vector<std::array<uint8_t, 3>> session_ouis;

void session_filter_clear()
{
    session_ssids.clear();
    session_wifi_macs.clear();
    session_ble_macs.clear();
    session_ouis.clear();
}

bool session_filter_add_ssid(const char* ssid)
{
    if (!ssid || !*ssid) return false;
    
    // Check if already exists
    std::string ssid_str(ssid);
    for (const auto& s : session_ssids) {
        if (s == ssid_str) return true; // Already exists
    }
    
    session_ssids.push_back(ssid_str);
    return true;
}

bool session_filter_add_mac_wifi(const uint8_t mac[6])
{
    if (!mac) return false;
    
    std::array<uint8_t, 6> mac_array;
    std::memcpy(mac_array.data(), mac, 6);
    
    // Check if already exists
    for (const auto& m : session_wifi_macs) {
        if (std::memcmp(m.data(), mac, 6) == 0) return true; // Already exists
    }
    
    session_wifi_macs.push_back(mac_array);
    return true;
}

bool session_filter_add_mac_ble(const uint8_t mac[6])
{
    if (!mac) return false;
    
    std::array<uint8_t, 6> mac_array;
    std::memcpy(mac_array.data(), mac, 6);
    
    // Check if already exists
    for (const auto& m : session_ble_macs) {
        if (std::memcmp(m.data(), mac, 6) == 0) return true; // Already exists
    }
    
    session_ble_macs.push_back(mac_array);
    return true;
}

bool session_filter_add_oui(const uint8_t oui[3])
{
    if (!oui) return false;
    
    std::array<uint8_t, 3> oui_array;
    std::memcpy(oui_array.data(), oui, 3);
    
    // Check if already exists
    for (const auto& o : session_ouis) {
        if (std::memcmp(o.data(), oui, 3) == 0) return true; // Already exists
    }
    
    session_ouis.push_back(oui_array);
    return true;
}

bool session_filter_is_known_wifi(const char* ssid_nullable, 
                                   const uint8_t mac_a_nullable[6], 
                                   const uint8_t mac_b_nullable[6])
{
    // Check SSID (case-sensitive exact match)
    if (ssid_nullable && *ssid_nullable) {
        std::string ssid_str(ssid_nullable);
        for (const auto& s : session_ssids) {
            if (s == ssid_str) return true;
        }
    }
    
    // Check MAC addresses (full 6-byte match)
    if (mac_a_nullable) {
        for (const auto& m : session_wifi_macs) {
            if (std::memcmp(m.data(), mac_a_nullable, 6) == 0) return true;
        }
        
        // Check OUI (first 3 bytes)
        for (const auto& o : session_ouis) {
            if (std::memcmp(o.data(), mac_a_nullable, 3) == 0) return true;
        }
    }
    
    if (mac_b_nullable) {
        for (const auto& m : session_wifi_macs) {
            if (std::memcmp(m.data(), mac_b_nullable, 6) == 0) return true;
        }
        
        // Check OUI (first 3 bytes)
        for (const auto& o : session_ouis) {
            if (std::memcmp(o.data(), mac_b_nullable, 3) == 0) return true;
        }
    }
    
    return false;
}

bool session_filter_is_known_ble(const uint8_t mac[6])
{
    if (!mac) return false;
    
    // Check full MAC match
    for (const auto& m : session_ble_macs) {
        if (std::memcmp(m.data(), mac, 6) == 0) return true;
    }
    
    // Check OUI (first 3 bytes)
    for (const auto& o : session_ouis) {
        if (std::memcmp(o.data(), mac, 3) == 0) return true;
    }
    
    return false;
}
