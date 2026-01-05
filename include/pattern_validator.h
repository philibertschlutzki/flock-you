/**
 * @file pattern_validator.h
 * @brief Validierung von Erkennungsmustern mit Wildcard-Support
 * 
 * Stellt sicher, dass Patterns eine Mindest-Spezifität haben und nicht
 * zu allgemein sind (z.B. verhindert reine `*` Patterns = match-all).
 */

#pragma once

#include <ctype.h>
#include <string.h>

/**
 * @brief Pattern-Typen für Validierung
 */
enum PatternType {
    PATTERN_TYPE_SSID,      ///< WiFi SSID oder BLE Name
    PATTERN_TYPE_MAC,       ///< MAC-Adresse
    PATTERN_TYPE_UUID       ///< BLE Service UUID
};

/**
 * @brief Zählt Literal-Zeichen (nicht-Wildcard) in einem Pattern
 * 
 * @param pattern Pattern-String
 * @return Anzahl der Literal-Zeichen (ohne `*`)
 */
static inline int count_literal_chars(const char* pattern)
{
    if (!pattern) return 0;
    
    int count = 0;
    while (*pattern) {
        if (*pattern != '*') {
            count++;
        }
        pattern++;
    }
    return count;
}

/**
 * @brief Zählt vollständige Hex-Byte-Paare in einem MAC-Pattern
 * 
 * Ein vollständiges Byte ist z.B. "aa:" (2 Hex-Zeichen + Doppelpunkt)
 * 
 * @param pattern MAC-Pattern
 * @return Anzahl vollständiger Bytes
 */
static inline int count_complete_mac_bytes(const char* pattern)
{
    if (!pattern) return 0;
    
    int count = 0;
    const char* p = pattern;
    
    while (*p) {
        // Prüfe auf Hex-Zeichen-Paar gefolgt von ':' oder Ende
        if (isxdigit(p[0]) && p[1] != '\0' && isxdigit(p[1])) {
            if (p[2] == ':' || p[2] == '\0') {
                count++;
                p += 2;
                if (*p == ':') p++;
            } else {
                p++;
            }
        } else if (*p == '*') {
            // Wildcard überspringen
            p++;
            if (*p == ':') p++;
        } else {
            p++;
        }
    }
    
    return count;
}

/**
 * @brief Zählt zusammenhängende Hex-Zeichen in einem UUID-Pattern
 * 
 * @param pattern UUID-Pattern
 * @return Anzahl zusammenhängender Hex-Zeichen
 */
static inline int count_consecutive_hex_chars(const char* pattern)
{
    if (!pattern) return 0;
    
    int max_consecutive = 0;
    int current_consecutive = 0;
    
    while (*pattern) {
        if (isxdigit(*pattern)) {
            current_consecutive++;
            if (current_consecutive > max_consecutive) {
                max_consecutive = current_consecutive;
            }
        } else {
            // Reset counter on any non-hex character (including '-', ':', etc.)
            current_consecutive = 0;
        }
        pattern++;
    }
    
    return max_consecutive;
}

/**
 * @brief Validiert ein Erkennungsmuster
 * 
 * Prüft ob Pattern die Mindest-Spezifität erfüllt:
 * - SSID/BLE Name: Mindestens 3 Literal-Zeichen
 * - MAC: Mindestens 1 vollständiges Byte (z.B. "aa:")
 * - UUID: Mindestens 8 zusammenhängende Hex-Zeichen
 * - Pattern darf nicht nur aus `*` bestehen
 * 
 * @param pattern Zu validierendes Pattern
 * @param type Pattern-Typ (SSID, MAC, UUID)
 * @return true wenn gültig, false wenn ungültig
 * 
 * @example
 * // Gültige Patterns:
 * validate_pattern("RAVEN-*", PATTERN_TYPE_SSID) -> true
 * validate_pattern("aa:bb:*", PATTERN_TYPE_MAC) -> true
 * validate_pattern("00003100-*", PATTERN_TYPE_UUID) -> true
 * 
 * // Ungültige Patterns:
 * validate_pattern("*", PATTERN_TYPE_SSID) -> false (nur Wildcard)
 * validate_pattern("a*", PATTERN_TYPE_SSID) -> false (zu kurz)
 * validate_pattern("*", PATTERN_TYPE_MAC) -> false (kein vollständiges Byte)
 */
static inline bool validate_pattern(const char* pattern, PatternType type)
{
    if (!pattern || !*pattern) {
        // Leere Patterns sind erlaubt (für Allowlist)
        return true;
    }

    // Pattern darf nicht nur aus * bestehen
    bool has_non_wildcard = false;
    for (const char* p = pattern; *p; p++) {
        if (*p != '*') {
            has_non_wildcard = true;
            break;
        }
    }
    if (!has_non_wildcard) {
        return false; // Nur `*` oder `**` etc. ist ungültig
    }

    switch (type) {
        case PATTERN_TYPE_SSID: {
            // Mindestens 3 Literal-Zeichen
            int literals = count_literal_chars(pattern);
            return literals >= 3;
        }
        
        case PATTERN_TYPE_MAC: {
            // Mindestens 1 vollständiges Byte
            int complete_bytes = count_complete_mac_bytes(pattern);
            return complete_bytes >= 1;
        }
        
        case PATTERN_TYPE_UUID: {
            // Mindestens 8 zusammenhängende Hex-Zeichen
            int hex_chars = count_consecutive_hex_chars(pattern);
            return hex_chars >= 8;
        }
        
        default:
            return false;
    }
}

/**
 * @brief Gibt eine Fehlerbeschreibung für ungültige Patterns zurück
 * 
 * @param pattern Ungültiges Pattern
 * @param type Pattern-Typ
 * @return Fehlerbeschreibung als String
 */
static inline const char* get_validation_error(const char* pattern, PatternType type)
{
    if (!pattern || !*pattern) {
        return "Empty pattern";
    }

    // Prüfe auf reines Wildcard-Pattern
    bool has_non_wildcard = false;
    for (const char* p = pattern; *p; p++) {
        if (*p != '*') {
            has_non_wildcard = true;
            break;
        }
    }
    if (!has_non_wildcard) {
        return "Pattern contains only wildcards (match-all not allowed)";
    }

    switch (type) {
        case PATTERN_TYPE_SSID: {
            int literals = count_literal_chars(pattern);
            if (literals < 3) {
                return "SSID/Name pattern must have at least 3 literal characters";
            }
            break;
        }
        
        case PATTERN_TYPE_MAC: {
            int complete_bytes = count_complete_mac_bytes(pattern);
            if (complete_bytes < 1) {
                return "MAC pattern must have at least 1 complete byte (e.g., 'aa:')";
            }
            break;
        }
        
        case PATTERN_TYPE_UUID: {
            int hex_chars = count_consecutive_hex_chars(pattern);
            if (hex_chars < 8) {
                return "UUID pattern must have at least 8 consecutive hex characters";
            }
            break;
        }
    }

    return "Unknown validation error";
}
