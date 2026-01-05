#!/bin/bash
# Validation script for blacklist-based filtering refactoring

echo "=========================================="
echo "Blacklist Refactoring Validation"
echo "=========================================="
echo ""

ERRORS=0
WARNINGS=0

# Check that old files are removed
echo "1. Checking old filter files are removed..."
OLD_FILES=(
    "include/detection_patterns.h"
    "include/wildcard_match.h"
    "include/pattern_validator.h"
    "include/pattern_startup_check.h"
    "src/pattern_startup_check.cpp"
    "test/test_wildcard_matching.cpp"
)

for file in "${OLD_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "   ✗ FAIL: $file still exists"
        ERRORS=$((ERRORS + 1))
    else
        echo "   ✓ PASS: $file removed"
    fi
done

# Check that new files exist
echo ""
echo "2. Checking new blacklist files exist..."
NEW_FILES=(
    "include/blacklist.h"
    "src/blacklist.cpp"
    "include/session_filter.h"
    "src/session_filter.cpp"
    "include/known_blacklist_generated.h"
    "src/known_blacklist_generated.cpp"
    "tools/generate_blacklist_from_csv.py"
)

for file in "${NEW_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "   ✓ PASS: $file exists"
    else
        echo "   ✗ FAIL: $file missing"
        ERRORS=$((ERRORS + 1))
    fi
done

# Check that main.cpp doesn't include old headers
echo ""
echo "3. Checking main.cpp doesn't include old headers..."
if grep -q "pattern_startup_check.h" src/main.cpp; then
    echo "   ✗ FAIL: main.cpp still includes pattern_startup_check.h"
    ERRORS=$((ERRORS + 1))
else
    echo "   ✓ PASS: pattern_startup_check.h not included"
fi

if grep -q "validate_all_patterns" src/main.cpp; then
    echo "   ✗ FAIL: main.cpp still calls validate_all_patterns()"
    ERRORS=$((ERRORS + 1))
else
    echo "   ✓ PASS: validate_all_patterns() not called"
fi

# Check that wifi_sniffer.cpp uses new blacklist
echo ""
echo "4. Checking wifi_sniffer.cpp uses blacklist..."
if grep -q "#include \"blacklist.h\"" src/wifi_sniffer.cpp; then
    echo "   ✓ PASS: wifi_sniffer.cpp includes blacklist.h"
else
    echo "   ✗ FAIL: wifi_sniffer.cpp doesn't include blacklist.h"
    ERRORS=$((ERRORS + 1))
fi

if grep -q "is_known_wifi" src/wifi_sniffer.cpp; then
    echo "   ✓ PASS: wifi_sniffer.cpp uses is_known_wifi()"
else
    echo "   ✗ FAIL: wifi_sniffer.cpp doesn't use is_known_wifi()"
    ERRORS=$((ERRORS + 1))
fi

if grep -q "detection_patterns.h" src/wifi_sniffer.cpp; then
    echo "   ✗ FAIL: wifi_sniffer.cpp still includes detection_patterns.h"
    ERRORS=$((ERRORS + 1))
else
    echo "   ✓ PASS: detection_patterns.h not included"
fi

# Check that ble_scanner.cpp uses new blacklist
echo ""
echo "5. Checking ble_scanner.cpp uses blacklist..."
if grep -q "#include \"blacklist.h\"" src/ble_scanner.cpp; then
    echo "   ✓ PASS: ble_scanner.cpp includes blacklist.h"
else
    echo "   ✗ FAIL: ble_scanner.cpp doesn't include blacklist.h"
    ERRORS=$((ERRORS + 1))
fi

if grep -q "is_known_ble" src/ble_scanner.cpp; then
    echo "   ✓ PASS: ble_scanner.cpp uses is_known_ble()"
else
    echo "   ✗ FAIL: ble_scanner.cpp doesn't use is_known_ble()"
    ERRORS=$((ERRORS + 1))
fi

# Check platformio.ini
echo ""
echo "6. Checking platformio.ini..."
if grep -q "ENABLE_WILDCARD_VALIDATION" platformio.ini; then
    echo "   ⚠ WARNING: platformio.ini still has ENABLE_WILDCARD_VALIDATION"
    WARNINGS=$((WARNINGS + 1))
else
    echo "   ✓ PASS: ENABLE_WILDCARD_VALIDATION removed"
fi

# Check generated files
echo ""
echo "7. Checking generated blacklist files..."
if [ -f "src/known_blacklist_generated.cpp" ]; then
    LINE_COUNT=$(wc -l < src/known_blacklist_generated.cpp)
    if [ "$LINE_COUNT" -gt 1000 ]; then
        echo "   ✓ PASS: Blacklist generated ($LINE_COUNT lines)"
    else
        echo "   ⚠ WARNING: Blacklist seems small ($LINE_COUNT lines)"
        WARNINGS=$((WARNINGS + 1))
    fi
else
    echo "   ✗ FAIL: Blacklist not generated"
    ERRORS=$((ERRORS + 1))
fi

# Check documentation
echo ""
echo "8. Checking documentation..."
if [ -f "BLACKLIST_REFACTORING.md" ]; then
    echo "   ✓ PASS: BLACKLIST_REFACTORING.md exists"
else
    echo "   ⚠ WARNING: BLACKLIST_REFACTORING.md missing"
    WARNINGS=$((WARNINGS + 1))
fi

if [ -f "tools/README.md" ]; then
    echo "   ✓ PASS: tools/README.md exists"
else
    echo "   ⚠ WARNING: tools/README.md missing"
    WARNINGS=$((WARNINGS + 1))
fi

# Summary
echo ""
echo "=========================================="
echo "Validation Summary"
echo "=========================================="
echo "Errors: $ERRORS"
echo "Warnings: $WARNINGS"

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    echo ""
    echo "✓ ALL CHECKS PASSED"
    echo "The refactoring is complete and correct."
    exit 0
elif [ $ERRORS -eq 0 ]; then
    echo ""
    echo "⚠ PASSED WITH WARNINGS"
    echo "The refactoring is mostly complete, but review warnings above."
    exit 0
else
    echo ""
    echo "✗ VALIDATION FAILED"
    echo "Please fix the errors listed above."
    exit 1
fi
