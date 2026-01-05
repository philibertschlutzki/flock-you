/**
 * @file wildcard_match.h
 * @brief Wildcard-Matching Funktionen für Erkennungsmuster
 * 
 * Unterstützt case-insensitives Wildcard-Matching mit `*` (beliebige Zeichenfolge).
 * Alle Funktionen sind embedded-tauglich (kein Heap, stack-basiert).
 */

#pragma once

#include <ctype.h>
#include <string.h>

/**
 * @brief Case-insensitives Wildcard-Matching für SSID und BLE-Namen
 * 
 * Wildcard-Semantik:
 * - `*` = beliebige Zeichenfolge (inkl. leer)
 * - Alle Vergleiche sind case-insensitiv
 * 
 * Algorithmus: Iteratives Backtracking ohne Rekursion (embedded-tauglich)
 * 
 * @param pattern Wildcard-Pattern (z.B. "RAVEN-*", "*-Guest")
 * @param text Zu prüfender Text (z.B. SSID oder BLE Name)
 * @return true wenn Pattern matcht, sonst false
 * 
 * @example
 * wildcard_match_ci("Flock*", "Flock-123") -> true
 * wildcard_match_ci("*Guest", "MyNetwork-Guest") -> true
 * wildcard_match_ci("RAVEN-*", "raven-xyz") -> true (case-insensitiv)
 */
static inline bool wildcard_match_ci(const char* pattern, const char* text)
{
    if (!pattern || !text) return false;

    // Helper für case-insensitiven Vergleich
    auto lower = [](char c) -> char {
        return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
    };

    const char* star = nullptr;
    const char* ss = text;

    while (*text) {
        char pc = lower(*pattern);
        char sc = lower(*text);

        if (pc == sc) {
            pattern++;
            text++;
            continue;
        }
        if (pc == '*') {
            star = pattern++;
            ss = text;
            continue;
        }
        if (star) {
            pattern = star + 1;
            text = ++ss;
            continue;
        }
        return false;
    }

    // Alle verbleibenden * im Pattern überspringen
    while (*pattern == '*') pattern++;
    return *pattern == '\0';
}

/**
 * @brief Wildcard-Matching für MAC-Adressen
 * 
 * Format: `aa:bb:cc:dd:ee:ff` oder Präfix-Patterns wie `aa:bb:*`, `58:8e:*`
 * 
 * Wildcard-Semantik:
 * - `*` ersetzt beliebig viele Zeichen (typischerweise ganze Bytes)
 * - Vergleich ist case-insensitiv
 * 
 * @param pattern MAC-Pattern (z.B. "aa:bb:cc:*", "58:8e:*")
 * @param mac MAC-Adresse als String (z.B. "aa:bb:cc:dd:ee:ff")
 * @return true wenn Pattern matcht, sonst false
 * 
 * @example
 * wildcard_match_mac("aa:bb:cc:*", "aa:bb:cc:dd:ee:ff") -> true
 * wildcard_match_mac("58:8e:*", "58:8e:81:12:34:56") -> true
 * wildcard_match_mac("*:*:*:dd:ee:ff", "11:22:33:dd:ee:ff") -> true
 */
static inline bool wildcard_match_mac(const char* pattern, const char* mac)
{
    if (!pattern || !mac) return false;

    // Helper für case-insensitiven Vergleich
    auto lower = [](char c) -> char {
        return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
    };

    const char* star = nullptr;
    const char* ss = mac;

    while (*mac) {
        char pc = lower(*pattern);
        char sc = lower(*mac);

        if (pc == sc) {
            pattern++;
            mac++;
            continue;
        }
        if (pc == '*') {
            star = pattern++;
            ss = mac;
            continue;
        }
        if (star) {
            pattern = star + 1;
            mac = ++ss;
            continue;
        }
        return false;
    }

    // Alle verbleibenden * im Pattern überspringen
    while (*pattern == '*') pattern++;
    return *pattern == '\0';
}

/**
 * @brief Wildcard-Matching für UUIDs (128-bit Format)
 * 
 * Format: Standard UUID `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`
 * 
 * Wildcard-Semantik:
 * - `*` ersetzt beliebige Hex-Zeichen oder Abschnitte
 * - Vergleich ist case-insensitiv
 * 
 * @param pattern UUID-Pattern (z.B. "00003100-*", "*-00805f9b34fb")
 * @param uuid UUID-String im Standard-Format
 * @return true wenn Pattern matcht, sonst false
 * 
 * @example
 * wildcard_match_uuid("00003100-*", "00003100-0000-1000-8000-00805f9b34fb") -> true
 * wildcard_match_uuid("*-00805f9b34fb", "00003100-0000-1000-8000-00805f9b34fb") -> true
 */
static inline bool wildcard_match_uuid(const char* pattern, const char* uuid)
{
    if (!pattern || !uuid) return false;

    // Helper für case-insensitiven Vergleich
    auto lower = [](char c) -> char {
        return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
    };

    const char* star = nullptr;
    const char* ss = uuid;

    while (*uuid) {
        char pc = lower(*pattern);
        char sc = lower(*uuid);

        if (pc == sc) {
            pattern++;
            uuid++;
            continue;
        }
        if (pc == '*') {
            star = pattern++;
            ss = uuid;
            continue;
        }
        if (star) {
            pattern = star + 1;
            uuid = ++ss;
            continue;
        }
        return false;
    }

    // Alle verbleibenden * im Pattern überspringen
    while (*pattern == '*') pattern++;
    return *pattern == '\0';
}
