/**
 * @file state.cpp
 * @brief Implementierung der zentralen globalen State-Variablen
 */

#include "state.h"

// ============================================================================
// DETECTION STATE - DEFINITIONS
// ============================================================================

bool triggered = false;
bool device_in_range = false;
unsigned long last_detection_time = 0;
unsigned long last_heartbeat = 0;
