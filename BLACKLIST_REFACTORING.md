# Refactoring: Blacklist-Based Filtering System

## Summary

This refactoring replaces the legacy pattern-based filtering system with a blacklist-based approach. The new system reports **only unknown devices** - devices not in the blacklist are reported as detections.

## Changes Made

### 1. Removed Legacy Filter System

**Deleted files:**
- `include/detection_patterns.h` - Pattern definitions (wildcards, SSIDs, MACs, UUIDs)
- `include/wildcard_match.h` - Wildcard matching functions
- `include/pattern_validator.h` - Pattern validation logic
- `include/pattern_startup_check.h` - Boot-time pattern validation
- `src/pattern_startup_check.cpp` - Pattern validation implementation
- `test/test_wildcard_matching.cpp` - Unit tests for wildcard matching

**Removed functionality:**
- Pattern/wildcard filtering (no more `*` wildcards)
- Allowlist/Legacy filter modes
- Raven Service UUID filtering
- BLE Device Name pattern matching
- Pattern validation at boot

### 2. New Blacklist System

**Created files:**

#### Generator
- `tools/generate_blacklist_from_csv.py` - Python script to generate blacklists from CSV data
  - Input: CSV files in `datasets/` directory
  - Output: `known_blacklist_generated.h` and `known_blacklist_generated.cpp`

#### Generated Blacklists (Compile-time)
- `include/known_blacklist_generated.h` - Header declarations
- `src/known_blacklist_generated.cpp` - Blacklist data arrays
  - WiFi SSIDs (8,601 entries, case-sensitive)
  - WiFi MACs (10,567 entries, 6-byte comparison)
  - BLE MACs (12,544 entries, 6-byte comparison)
  - OUIs (17,416 entries, first 3 bytes)

#### Session Filter (RAM-only)
- `include/session_filter.h` - Session filter API
- `src/session_filter.cpp` - Runtime filter implementation
  - In-memory storage (std::vector)
  - API for adding SSIDs, MACs, OUIs at runtime
  - Not persisted (reset on reboot)
  - Hook for future WebGUI/API integration

#### Blacklist Checker
- `include/blacklist.h` - Unified blacklist checking API
- `src/blacklist.cpp` - Blacklist checking implementation
  - Combines compile-time and session filters
  - `is_known_wifi()` - Check if WiFi device is known
  - `is_known_ble()` - Check if BLE device is known

### 3. Updated Detection Logic

**Modified files:**
- `src/wifi_sniffer.cpp` - WiFi sniffer now uses blacklist
  - Removed: Pattern matching, wildcard checks
  - Added: Blacklist checking via `is_known_wifi()`
  - Reports: `probe_request_unknown` and `beacon_unknown` for unknown devices
  
- `src/ble_scanner.cpp` - BLE scanner now uses blacklist
  - Removed: Device name patterns, Raven UUID checks, MAC pattern matching
  - Added: Blacklist checking via `is_known_ble()`
  - Reports: `unknown` detection type for unknown devices
  - Note: Device name is included in output for info but not used for filtering

- `include/wifi_sniffer.h` - Updated header
  - Removed: `wifi_check_ssid_pattern()`, `wifi_check_mac_prefix()`
  
- `include/ble_scanner.h` - Updated header
  - Removed: `ble_check_device_name_pattern()`

- `src/main.cpp` - Removed pattern validation
  - Removed: `#include "pattern_startup_check.h"`
  - Removed: `validate_all_patterns()` call

### 4. Build Configuration

**Modified files:**
- `platformio.ini` - Updated build configuration
  - Removed: `-DENABLE_WILDCARD_VALIDATION=1`
  - Removed: Filter mode documentation
  - Added: Blacklist generation documentation

- `BUILD.md` - Updated build instructions
  - Added: Blacklist generation section
  - Updated: Module compilation table
  - Added: Troubleshooting for missing blacklist files

**Created files:**
- `tools/README.md` - Documentation for generator script

## Migration Guide

### For Developers

**Before:**
```cpp
// Old pattern-based detection
if (wifi_check_ssid_pattern(ssid) || wifi_check_mac_prefix(mac)) {
    // Trigger detection
}
```

**After:**
```cpp
// New blacklist-based detection
if (is_known_wifi(ssid, mac_a, mac_b)) {
    // Skip - device is known
    return;
}
// Device is unknown - report it
output_wifi_detection_json(ssid, mac, rssi, "probe_request_unknown");
```

### For Users

**Build process:**

1. Generate blacklist (only needed if CSV data changes):
   ```bash
   python3 tools/generate_blacklist_from_csv.py
   ```

2. Build firmware:
   ```bash
   pio run -e lilygo_tdisplay_s3
   ```

**Runtime behavior:**

- **Before:** Devices matching patterns triggered detection
- **After:** Only unknown devices (not in blacklist) trigger detection

## Technical Details

### SSID Matching
- **Case-sensitive** exact string comparison
- Hidden/empty SSIDs: SSID check is skipped, only MAC/OUI checked
- No wildcards or patterns

### MAC Matching
- Full 6-byte comparison against known MACs
- OUI (first 3 bytes) comparison against known OUIs
- No string-based patterns or wildcards

### BLE Detection
- MAC-based filtering only
- Device names are included in output but not used for filtering
- No Raven UUID filtering
- No device name patterns

### Session Filter API

The session filter provides hooks for runtime filtering (e.g., via WebGUI):

```cpp
// Clear all session filters
session_filter_clear();

// Add to session filter
session_filter_add_ssid("MyNetwork");
session_filter_add_mac_wifi(mac);
session_filter_add_mac_ble(mac);
session_filter_add_oui(oui);

// Check session filter (used internally by blacklist.cpp)
bool known = session_filter_is_known_wifi(ssid, mac_a, mac_b);
```

**Note:** Session filters are RAM-only and reset on reboot. They are intended for temporary runtime filtering via WebGUI/API.

## Performance Impact

### Compile Time
- **Before:** Minimal - small pattern arrays
- **After:** Larger - ~49,000 lines in generated file
- **Build time:** Negligible increase (arrays are simple data)

### Memory Usage
- **Before:** ~1-2 KB (small pattern arrays)
- **After:** ~500-600 KB (large blacklist arrays in flash)
- **Note:** Blacklist data is stored in flash, not RAM

### Runtime Performance
- **Before:** Linear search through pattern arrays + wildcard matching
- **After:** Linear search through blacklist arrays (exact comparison)
- **Performance:** Similar for typical use cases (blacklist is in flash)

## Validation

### Build Validation
```bash
# Verify all old files are removed
ls include/detection_patterns.h  # Should fail
ls include/wildcard_match.h      # Should fail

# Verify new files exist
ls include/blacklist.h            # Should succeed
ls src/blacklist.cpp              # Should succeed
ls include/known_blacklist_generated.h  # Should succeed

# Build project
pio run -e lilygo_tdisplay_s3
```

### Runtime Validation
After flashing firmware, verify:
- WiFi sniffer reports "probe_request_unknown" and "beacon_unknown"
- BLE scanner reports "unknown" detection type
- No pattern validation messages at boot
- Only unknown devices trigger detection

## Known Issues / Limitations

1. **Large binary size:** Blacklist data increases flash usage by ~500 KB
2. **Static blacklist:** Compile-time blacklist cannot be updated without reflashing
3. **Session filter:** Not persisted, resets on reboot (by design)
4. **No wildcards:** Cannot match partial SSIDs or MAC patterns

## Future Enhancements

1. **Persistent session filter:** Save runtime filters to SPIFFS/LittleFS
2. **WebGUI integration:** Add/remove devices from session filter via web interface
3. **API integration:** REST API for managing session filters
4. **Compressed blacklist:** Use hash tables or bloom filters for smaller footprint
5. **Dynamic blacklist updates:** OTA updates for blacklist data

## References

- Issue: [Refactor-Goal: Remove legacy filter mechanisms]
- Generator script: `tools/generate_blacklist_from_csv.py`
- Build documentation: `BUILD.md`
- Tool documentation: `tools/README.md`
