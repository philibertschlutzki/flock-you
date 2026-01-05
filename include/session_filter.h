/**
 * @file session_filter.h
 * @brief RAM-only session filter for runtime blacklist management
 * 
 * Provides in-memory filtering hooks for WebGUI/API integration.
 * Session filters are NOT persisted - they reset on reboot.
 * 
 * Usage:
 *  - Add devices to filter at runtime via API/WebGUI
 *  - Check if devices are in session filter
 *  - Clear session filter when needed
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Clear all session filters (SSIDs, MACs, OUIs)
 */
void session_filter_clear();

/**
 * @brief Add SSID to session filter (case-sensitive)
 * 
 * @param ssid SSID to add (case-sensitive, null-terminated)
 * @return true if added successfully, false on error
 */
bool session_filter_add_ssid(const char* ssid);

/**
 * @brief Add WiFi MAC address to session filter
 * 
 * @param mac MAC address (6 bytes)
 * @return true if added successfully, false on error
 */
bool session_filter_add_mac_wifi(const uint8_t mac[6]);

/**
 * @brief Add BLE MAC address to session filter
 * 
 * @param mac MAC address (6 bytes)
 * @return true if added successfully, false on error
 */
bool session_filter_add_mac_ble(const uint8_t mac[6]);

/**
 * @brief Add OUI (manufacturer prefix) to session filter
 * 
 * @param oui OUI prefix (first 3 bytes of MAC)
 * @return true if added successfully, false on error
 */
bool session_filter_add_oui(const uint8_t oui[3]);

/**
 * @brief Check if WiFi device is in session filter
 * 
 * Checks if SSID or MAC matches any entry in session filter.
 * 
 * @param ssid_nullable SSID (nullable, case-sensitive)
 * @param mac_a_nullable First MAC address (nullable, 6 bytes)
 * @param mac_b_nullable Second MAC address (nullable, 6 bytes)
 * @return true if device is in session filter, false otherwise
 */
bool session_filter_is_known_wifi(const char* ssid_nullable, 
                                   const uint8_t mac_a_nullable[6], 
                                   const uint8_t mac_b_nullable[6]);

/**
 * @brief Check if BLE device is in session filter
 * 
 * Checks if MAC matches any entry in session filter.
 * 
 * @param mac MAC address (6 bytes)
 * @return true if device is in session filter, false otherwise
 */
bool session_filter_is_known_ble(const uint8_t mac[6]);
