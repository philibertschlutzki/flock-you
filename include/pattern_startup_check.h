/**
 * @file pattern_startup_check.h
 * @brief Startup-Validierung aller Erkennungsmuster
 */

#pragma once

/**
 * @brief Validiert alle Erkennungsmuster beim Boot
 * 
 * Sollte in main.cpp vor dem Start der Scanner aufgerufen werden.
 * Loggt ungültige Patterns und kann optional Boot abbrechen (STRICT_PATTERN_VALIDATION).
 */
void validate_all_patterns();
