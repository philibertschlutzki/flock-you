/**
 * @file test_wildcard_matching.cpp
 * @brief Unit Tests für Wildcard-Matching und Pattern-Validierung
 * 
 * PlatformIO Unity Test Framework
 */

#include <unity.h>
#include "../include/wildcard_match.h"
#include "../include/pattern_validator.h"

// ============================================================================
// Tests für wildcard_match_ci() - SSID/BLE Name Matching
// ============================================================================

void test_wildcard_match_ci_exact_match(void)
{
    TEST_ASSERT_TRUE(wildcard_match_ci("Flock", "Flock"));
    TEST_ASSERT_TRUE(wildcard_match_ci("RAVEN", "RAVEN"));
}

void test_wildcard_match_ci_case_insensitive(void)
{
    TEST_ASSERT_TRUE(wildcard_match_ci("flock", "FLOCK"));
    TEST_ASSERT_TRUE(wildcard_match_ci("RAVEN", "raven"));
    TEST_ASSERT_TRUE(wildcard_match_ci("RaVeN", "rAvEn"));
}

void test_wildcard_match_ci_prefix_wildcard(void)
{
    TEST_ASSERT_TRUE(wildcard_match_ci("Flock*", "Flock"));
    TEST_ASSERT_TRUE(wildcard_match_ci("Flock*", "Flock-123"));
    TEST_ASSERT_TRUE(wildcard_match_ci("Flock*", "FlockCamera"));
    TEST_ASSERT_TRUE(wildcard_match_ci("RAVEN-*", "RAVEN-123"));
    TEST_ASSERT_TRUE(wildcard_match_ci("RAVEN-*", "raven-xyz"));
}

void test_wildcard_match_ci_suffix_wildcard(void)
{
    TEST_ASSERT_TRUE(wildcard_match_ci("*Guest", "Guest"));
    TEST_ASSERT_TRUE(wildcard_match_ci("*Guest", "MyNetwork-Guest"));
    TEST_ASSERT_TRUE(wildcard_match_ci("*-Guest", "Office-Guest"));
    TEST_ASSERT_FALSE(wildcard_match_ci("*Guest", "GuestNetwork"));
}

void test_wildcard_match_ci_middle_wildcard(void)
{
    TEST_ASSERT_TRUE(wildcard_match_ci("Flock*Camera", "FlockCamera"));
    TEST_ASSERT_TRUE(wildcard_match_ci("Flock*Camera", "FlockStreetCamera"));
    TEST_ASSERT_TRUE(wildcard_match_ci("Flock*Camera", "Flock_Camera"));
}

void test_wildcard_match_ci_multiple_wildcards(void)
{
    TEST_ASSERT_TRUE(wildcard_match_ci("*RAVEN*", "RAVEN"));
    TEST_ASSERT_TRUE(wildcard_match_ci("*RAVEN*", "MyRAVENDevice"));
    TEST_ASSERT_TRUE(wildcard_match_ci("*RAVEN*", "RAVEN-123"));
    TEST_ASSERT_TRUE(wildcard_match_ci("R*V*N", "RAVEN"));
}

void test_wildcard_match_ci_no_match(void)
{
    TEST_ASSERT_FALSE(wildcard_match_ci("Flock", "Penguin"));
    TEST_ASSERT_FALSE(wildcard_match_ci("RAVEN-*", "Flock-123"));
    TEST_ASSERT_FALSE(wildcard_match_ci("Flock*", "Penguin"));
}

void test_wildcard_match_ci_null_handling(void)
{
    TEST_ASSERT_FALSE(wildcard_match_ci(nullptr, "test"));
    TEST_ASSERT_FALSE(wildcard_match_ci("test", nullptr));
    TEST_ASSERT_FALSE(wildcard_match_ci(nullptr, nullptr));
}

void test_wildcard_match_ci_empty_strings(void)
{
    TEST_ASSERT_TRUE(wildcard_match_ci("", ""));
    TEST_ASSERT_TRUE(wildcard_match_ci("*", ""));
    TEST_ASSERT_TRUE(wildcard_match_ci("*", "anything"));
    TEST_ASSERT_FALSE(wildcard_match_ci("", "something"));
}

// ============================================================================
// Tests für wildcard_match_mac() - MAC-Adress Matching
// ============================================================================

void test_wildcard_match_mac_exact_match(void)
{
    TEST_ASSERT_TRUE(wildcard_match_mac("aa:bb:cc:dd:ee:ff", "aa:bb:cc:dd:ee:ff"));
    TEST_ASSERT_TRUE(wildcard_match_mac("58:8e:81:12:34:56", "58:8e:81:12:34:56"));
}

void test_wildcard_match_mac_case_insensitive(void)
{
    TEST_ASSERT_TRUE(wildcard_match_mac("aa:bb:cc:dd:ee:ff", "AA:BB:CC:DD:EE:FF"));
    TEST_ASSERT_TRUE(wildcard_match_mac("AA:BB:CC:DD:EE:FF", "aa:bb:cc:dd:ee:ff"));
}

void test_wildcard_match_mac_prefix_wildcard(void)
{
    TEST_ASSERT_TRUE(wildcard_match_mac("aa:bb:cc:*", "aa:bb:cc:dd:ee:ff"));
    TEST_ASSERT_TRUE(wildcard_match_mac("58:8e:*", "58:8e:81:12:34:56"));
    TEST_ASSERT_TRUE(wildcard_match_mac("58:8e:81:*", "58:8e:81:12:34:56"));
}

void test_wildcard_match_mac_suffix_wildcard(void)
{
    TEST_ASSERT_TRUE(wildcard_match_mac("*:dd:ee:ff", "aa:bb:cc:dd:ee:ff"));
    TEST_ASSERT_TRUE(wildcard_match_mac("*:*:*:dd:ee:ff", "11:22:33:dd:ee:ff"));
}

void test_wildcard_match_mac_middle_wildcard(void)
{
    TEST_ASSERT_TRUE(wildcard_match_mac("aa:*:ff", "aa:bb:ff"));
    TEST_ASSERT_TRUE(wildcard_match_mac("aa:*:cc:*:ff", "aa:bb:cc:dd:ff"));
}

void test_wildcard_match_mac_no_match(void)
{
    TEST_ASSERT_FALSE(wildcard_match_mac("aa:bb:cc:*", "aa:bb:dd:ee:ff:11"));
    TEST_ASSERT_FALSE(wildcard_match_mac("58:8e:*", "58:8f:81:12:34:56"));
}

void test_wildcard_match_mac_null_handling(void)
{
    TEST_ASSERT_FALSE(wildcard_match_mac(nullptr, "aa:bb:cc:dd:ee:ff"));
    TEST_ASSERT_FALSE(wildcard_match_mac("aa:bb:cc:*", nullptr));
}

// ============================================================================
// Tests für wildcard_match_uuid() - UUID Matching
// ============================================================================

void test_wildcard_match_uuid_exact_match(void)
{
    TEST_ASSERT_TRUE(wildcard_match_uuid(
        "00003100-0000-1000-8000-00805f9b34fb",
        "00003100-0000-1000-8000-00805f9b34fb"
    ));
}

void test_wildcard_match_uuid_case_insensitive(void)
{
    TEST_ASSERT_TRUE(wildcard_match_uuid(
        "00003100-0000-1000-8000-00805f9b34fb",
        "00003100-0000-1000-8000-00805F9B34FB"
    ));
}

void test_wildcard_match_uuid_prefix_wildcard(void)
{
    TEST_ASSERT_TRUE(wildcard_match_uuid(
        "00003100-*",
        "00003100-0000-1000-8000-00805f9b34fb"
    ));
    TEST_ASSERT_TRUE(wildcard_match_uuid(
        "00003200-*",
        "00003200-0000-1000-8000-00805f9b34fb"
    ));
}

void test_wildcard_match_uuid_suffix_wildcard(void)
{
    TEST_ASSERT_TRUE(wildcard_match_uuid(
        "*-00805f9b34fb",
        "00003100-0000-1000-8000-00805f9b34fb"
    ));
    TEST_ASSERT_TRUE(wildcard_match_uuid(
        "*-8000-00805f9b34fb",
        "00003100-0000-1000-8000-00805f9b34fb"
    ));
}

void test_wildcard_match_uuid_middle_wildcard(void)
{
    TEST_ASSERT_TRUE(wildcard_match_uuid(
        "00003100-*-00805f9b34fb",
        "00003100-0000-1000-8000-00805f9b34fb"
    ));
}

void test_wildcard_match_uuid_no_match(void)
{
    TEST_ASSERT_FALSE(wildcard_match_uuid(
        "00003100-*",
        "00003200-0000-1000-8000-00805f9b34fb"
    ));
}

void test_wildcard_match_uuid_null_handling(void)
{
    TEST_ASSERT_FALSE(wildcard_match_uuid(nullptr, "00003100-0000-1000-8000-00805f9b34fb"));
    TEST_ASSERT_FALSE(wildcard_match_uuid("00003100-*", nullptr));
}

// ============================================================================
// Tests für Pattern Validation
// ============================================================================

void test_validate_pattern_ssid_valid(void)
{
    TEST_ASSERT_TRUE(validate_pattern("RAVEN-*", PATTERN_TYPE_SSID));
    TEST_ASSERT_TRUE(validate_pattern("Flock*Camera", PATTERN_TYPE_SSID));
    TEST_ASSERT_TRUE(validate_pattern("*-Guest", PATTERN_TYPE_SSID));
    TEST_ASSERT_TRUE(validate_pattern("abc", PATTERN_TYPE_SSID));
    TEST_ASSERT_TRUE(validate_pattern("MyNetwork", PATTERN_TYPE_SSID));
}

void test_validate_pattern_ssid_invalid(void)
{
    TEST_ASSERT_FALSE(validate_pattern("*", PATTERN_TYPE_SSID));
    TEST_ASSERT_FALSE(validate_pattern("**", PATTERN_TYPE_SSID));
    TEST_ASSERT_FALSE(validate_pattern("a*", PATTERN_TYPE_SSID));
    TEST_ASSERT_FALSE(validate_pattern("ab*", PATTERN_TYPE_SSID));
    TEST_ASSERT_FALSE(validate_pattern("*a", PATTERN_TYPE_SSID));
}

void test_validate_pattern_mac_valid(void)
{
    TEST_ASSERT_TRUE(validate_pattern("aa:bb:cc:*", PATTERN_TYPE_MAC));
    TEST_ASSERT_TRUE(validate_pattern("58:8e:*", PATTERN_TYPE_MAC));
    TEST_ASSERT_TRUE(validate_pattern("aa:*", PATTERN_TYPE_MAC));
    TEST_ASSERT_TRUE(validate_pattern("*:dd:ee:ff", PATTERN_TYPE_MAC));
    TEST_ASSERT_TRUE(validate_pattern("aa:bb:cc:dd:ee:ff", PATTERN_TYPE_MAC));
}

void test_validate_pattern_mac_invalid(void)
{
    TEST_ASSERT_FALSE(validate_pattern("*", PATTERN_TYPE_MAC));
    TEST_ASSERT_FALSE(validate_pattern("**", PATTERN_TYPE_MAC));
    TEST_ASSERT_FALSE(validate_pattern("*:*:*:*:*:*", PATTERN_TYPE_MAC));
}

void test_validate_pattern_uuid_valid(void)
{
    TEST_ASSERT_TRUE(validate_pattern("00003100-*", PATTERN_TYPE_UUID));
    TEST_ASSERT_TRUE(validate_pattern("*-00805f9b34fb", PATTERN_TYPE_UUID));
    TEST_ASSERT_TRUE(validate_pattern("00003100-0000-1000-8000-00805f9b34fb", PATTERN_TYPE_UUID));
}

void test_validate_pattern_uuid_invalid(void)
{
    TEST_ASSERT_FALSE(validate_pattern("*", PATTERN_TYPE_UUID));
    TEST_ASSERT_FALSE(validate_pattern("**", PATTERN_TYPE_UUID));
    TEST_ASSERT_FALSE(validate_pattern("0000*", PATTERN_TYPE_UUID));
}

void test_validate_pattern_empty(void)
{
    // Leere Patterns sind erlaubt (für Allowlist)
    TEST_ASSERT_TRUE(validate_pattern("", PATTERN_TYPE_SSID));
    TEST_ASSERT_TRUE(validate_pattern("", PATTERN_TYPE_MAC));
    TEST_ASSERT_TRUE(validate_pattern("", PATTERN_TYPE_UUID));
}

// ============================================================================
// Test Runner Setup
// ============================================================================

void setUp(void)
{
    // Vor jedem Test ausgeführt
}

void tearDown(void)
{
    // Nach jedem Test ausgeführt
}

int runUnityTests(void)
{
    UNITY_BEGIN();
    
    // wildcard_match_ci Tests
    RUN_TEST(test_wildcard_match_ci_exact_match);
    RUN_TEST(test_wildcard_match_ci_case_insensitive);
    RUN_TEST(test_wildcard_match_ci_prefix_wildcard);
    RUN_TEST(test_wildcard_match_ci_suffix_wildcard);
    RUN_TEST(test_wildcard_match_ci_middle_wildcard);
    RUN_TEST(test_wildcard_match_ci_multiple_wildcards);
    RUN_TEST(test_wildcard_match_ci_no_match);
    RUN_TEST(test_wildcard_match_ci_null_handling);
    RUN_TEST(test_wildcard_match_ci_empty_strings);
    
    // wildcard_match_mac Tests
    RUN_TEST(test_wildcard_match_mac_exact_match);
    RUN_TEST(test_wildcard_match_mac_case_insensitive);
    RUN_TEST(test_wildcard_match_mac_prefix_wildcard);
    RUN_TEST(test_wildcard_match_mac_suffix_wildcard);
    RUN_TEST(test_wildcard_match_mac_middle_wildcard);
    RUN_TEST(test_wildcard_match_mac_no_match);
    RUN_TEST(test_wildcard_match_mac_null_handling);
    
    // wildcard_match_uuid Tests
    RUN_TEST(test_wildcard_match_uuid_exact_match);
    RUN_TEST(test_wildcard_match_uuid_case_insensitive);
    RUN_TEST(test_wildcard_match_uuid_prefix_wildcard);
    RUN_TEST(test_wildcard_match_uuid_suffix_wildcard);
    RUN_TEST(test_wildcard_match_uuid_middle_wildcard);
    RUN_TEST(test_wildcard_match_uuid_no_match);
    RUN_TEST(test_wildcard_match_uuid_null_handling);
    
    // Pattern Validation Tests
    RUN_TEST(test_validate_pattern_ssid_valid);
    RUN_TEST(test_validate_pattern_ssid_invalid);
    RUN_TEST(test_validate_pattern_mac_valid);
    RUN_TEST(test_validate_pattern_mac_invalid);
    RUN_TEST(test_validate_pattern_uuid_valid);
    RUN_TEST(test_validate_pattern_uuid_invalid);
    RUN_TEST(test_validate_pattern_empty);
    
    return UNITY_END();
}

/**
 * For Arduino framework
 */
void setup()
{
    delay(2000); // Service delay
    runUnityTests();
}

void loop()
{
    // Nothing to do here
}

/**
 * For native platform
 */
#ifdef UNITY_INCLUDE_CONFIG_H
int main(int argc, char **argv)
{
    return runUnityTests();
}
#endif
