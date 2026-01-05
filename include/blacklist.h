/**
 * @file blacklist.h
 * @brief Combined blacklist checking (compile-time + session)
 * 
 * Provides unified interface for checking if WiFi/BLE devices are known
 * (i.e., should be filtered out).
 * 
 * Checks against:
 *  - Compile-time blacklists (known_blacklist_generated.h)
 *  - Session filters (session_filter.h)
 * 
 * Matching rules:
 *  - SSID: exact case-sensitive match
 *  - MAC: full 6-byte comparison
 *  - OUI: first 3 bytes comparison
 */

#pragma once

#include <stdint.h>

/**
 * @brief Check if WiFi device is known (should be filtered)
 * 
 * Returns true if SSID or MAC matches compile-time blacklist or session filter.
 * 
 * SSID matching:
 *  - Exact case-sensitive comparison
 *  - Hidden/empty SSIDs: SSID check is skipped, only MAC/OUI checked
 * 
 * MAC matching:
 *  - Full 6-byte comparison against known WiFi MACs
 *  - OUI (first 3 bytes) comparison against known OUIs
 * 
 * @param ssid_nullable WiFi SSID (nullable, can be NULL or empty for hidden networks)
 * @param mac_a_nullable First MAC address (nullable, typically client/source MAC)
 * @param mac_b_nullable Second MAC address (nullable, typically BSSID)
 * @return true if device is known (should be filtered), false if unknown
 */
bool is_known_wifi(const char* ssid_nullable, 
                   const uint8_t mac_a_nullable[6], 
                   const uint8_t mac_b_nullable[6]);

/**
 * @brief Check if BLE device is known (should be filtered)
 * 
 * Returns true if MAC matches compile-time blacklist or session filter.
 * 
 * MAC matching:
 *  - Full 6-byte comparison against known BLE MACs
 *  - OUI (first 3 bytes) comparison against known OUIs
 * 
 * @param mac BLE MAC address (6 bytes, must not be NULL)
 * @return true if device is known (should be filtered), false if unknown
 */
bool is_known_ble(const uint8_t mac[6]);
