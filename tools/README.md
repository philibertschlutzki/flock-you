# Tools

## Blacklist Generator

### generate_blacklist_from_csv.py

Generates compile-time blacklist from CSV session data.

**Usage:**
```bash
python3 tools/generate_blacklist_from_csv.py
```

**Input:**
- CSV files in `datasets/` directory
- Columns used:
  - `type`: "BLE" for Bluetooth devices, or WiFi if not specified
  - `ssid`: WiFi SSID (case-sensitive)
  - `netid`: MAC address (format: `aa:bb:cc:dd:ee:ff`)

**Output:**
- `include/known_blacklist_generated.h` - Header declarations
- `src/known_blacklist_generated.cpp` - Blacklist data arrays

**Generated blacklists:**
- WiFi SSIDs (case-sensitive exact match)
- WiFi MAC addresses (6-byte comparison)
- BLE MAC addresses (6-byte comparison)
- OUIs (first 3 bytes of MAC, for both WiFi and BLE)

**Note:** The generated files are checked into the repository. You only need to run this script if you:
- Add new CSV files to `datasets/`
- Update existing CSV files
- Want to refresh the blacklist

After generating, rebuild the project:
```bash
pio run -e lilygo_tdisplay_s3
```
