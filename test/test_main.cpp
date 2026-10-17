/**
 * @file test_main.cpp
 * @brief CYD OS Unit Tests (Phase A)
 * 
 * Tests that can run on-device (native ESP32 tests via Unity).
 * These validate compile-time constants and basic logic.
 */

#include <Arduino.h>
#include <unity.h>
#include "pin_config.h"
#include "app_config.h"

// ==========================================================================
// Pin configuration tests
// ==========================================================================
void test_pin_definitions_not_conflicting() {
    // Display and touch should not share any pins
    TEST_ASSERT_NOT_EQUAL(PIN_LCD_SCK, PIN_TOUCH_CLK);
    TEST_ASSERT_NOT_EQUAL(PIN_LCD_MOSI, PIN_TOUCH_DIN);
    TEST_ASSERT_NOT_EQUAL(PIN_LCD_CS, PIN_TOUCH_CS);
    
    // SD should not share pins with display
    TEST_ASSERT_NOT_EQUAL(PIN_SD_SCK, PIN_LCD_SCK);
    TEST_ASSERT_NOT_EQUAL(PIN_SD_MOSI, PIN_LCD_MOSI);
    TEST_ASSERT_NOT_EQUAL(PIN_SD_CS, PIN_LCD_CS);
    
    // SD should not share pins with touch
    TEST_ASSERT_NOT_EQUAL(PIN_SD_SCK, PIN_TOUCH_CLK);
    TEST_ASSERT_NOT_EQUAL(PIN_SD_CS, PIN_TOUCH_CS);
}

void test_display_dimensions() {
    TEST_ASSERT_EQUAL(240, DISPLAY_WIDTH);
    TEST_ASSERT_EQUAL(320, DISPLAY_HEIGHT);
}

void test_input_only_pins_not_used_as_output() {
    // GPIO 34, 35, 36, 39 are input-only
    // They should not be used for LED, speaker, or any output function
    TEST_ASSERT_NOT_EQUAL(34, PIN_LED_RED);
    TEST_ASSERT_NOT_EQUAL(34, PIN_LED_GREEN);
    TEST_ASSERT_NOT_EQUAL(34, PIN_LED_BLUE);
    TEST_ASSERT_NOT_EQUAL(34, PIN_SPEAKER);
    
    TEST_ASSERT_NOT_EQUAL(35, PIN_LED_RED);
    TEST_ASSERT_NOT_EQUAL(35, PIN_LED_GREEN);
    TEST_ASSERT_NOT_EQUAL(35, PIN_LED_BLUE);
    
    TEST_ASSERT_NOT_EQUAL(36, PIN_LED_RED);
    TEST_ASSERT_NOT_EQUAL(36, PIN_LED_GREEN);
    TEST_ASSERT_NOT_EQUAL(36, PIN_LED_BLUE);
    
    TEST_ASSERT_NOT_EQUAL(39, PIN_LED_RED);
    TEST_ASSERT_NOT_EQUAL(39, PIN_LED_GREEN);
    TEST_ASSERT_NOT_EQUAL(39, PIN_LED_BLUE);
}

// ==========================================================================
// App configuration tests
// ==========================================================================
void test_config_constants() {
    TEST_ASSERT_GREATER_THAN(0, LVGL_DRAW_BUF_LINES);
    TEST_ASSERT_LESS_OR_EQUAL(DISPLAY_HEIGHT, LVGL_DRAW_BUF_LINES);
    
    TEST_ASSERT_GREATER_THAN(0, TOUCH_DEBOUNCE_MS);
    TEST_ASSERT_LESS_THAN(1000, TOUCH_DEBOUNCE_MS);
    
    TEST_ASSERT_GREATER_THAN(0, WIFI_CONNECT_TIMEOUT_MS);
    TEST_ASSERT_GREATER_THAN(0, HTTP_TIMEOUT_MS);
    
    TEST_ASSERT_GREATER_THAN(0, MAX_NOTES);
    TEST_ASSERT_GREATER_THAN(0, MAX_NOTE_LENGTH);
    TEST_ASSERT_GREATER_THAN(0, MAX_NOTIFICATIONS);
}

void test_memory_thresholds() {
    TEST_ASSERT_GREATER_THAN(HEAP_CRITICAL_THRESHOLD, HEAP_WARNING_THRESHOLD);
    TEST_ASSERT_GREATER_THAN(0, HEAP_CRITICAL_THRESHOLD);
}

void test_display_buffer_fits_in_memory() {
    // Single draw buffer size in bytes
    size_t bufSize = DISPLAY_WIDTH * LVGL_DRAW_BUF_LINES * sizeof(uint16_t);
    // Should be < 32KB to leave room for everything else
    TEST_ASSERT_LESS_THAN(32768, bufSize);
}

// ==========================================================================
// Test runner
// ==========================================================================
void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_pin_definitions_not_conflicting);
    RUN_TEST(test_display_dimensions);
    RUN_TEST(test_input_only_pins_not_used_as_output);
    RUN_TEST(test_config_constants);
    RUN_TEST(test_memory_thresholds);
    RUN_TEST(test_display_buffer_fits_in_memory);
    
    UNITY_END();
}

void loop() {
    // Tests complete
}
