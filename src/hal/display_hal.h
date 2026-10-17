/**
 * @file display_hal.h
 * @brief Display Hardware Abstraction Layer for CYD OS
 * 
 * Provides display initialization, LVGL flush callback, and backlight control.
 * Uses LovyanGFX as the underlying display driver.
 */

#pragma once

#include <lvgl.h>

// Forward declaration — defined in cyd_config.h
class LGFX_CYD;

/**
 * @brief Get the global LovyanGFX display instance
 * @return Pointer to the display instance
 */
LGFX_CYD* display_get_instance();


namespace CydOS {
namespace HAL {

/**
 * @brief Initialize the display hardware and LVGL display driver
 * 
 * Sets up:
 * - LovyanGFX display (ST7789 or ILI9341 based on config)
 * - LVGL display driver registration
 * - Draw buffer allocation (single buffer, partial rendering)
 * - Backlight PWM
 * 
 * @return true if display initialized successfully
 */
bool display_init();

/**
 * @brief Set backlight brightness
 * @param brightness 0-255 (0=off, 255=max)
 */
void display_set_brightness(uint8_t brightness);

/**
 * @brief Get current backlight brightness
 * @return Current brightness level 0-255
 */
uint8_t display_get_brightness();

/**
 * @brief Turn display off (backlight to 0)
 */
void display_off();

/**
 * @brief Turn display on (restore last brightness)
 */
void display_on();

/**
 * @brief Check if display is currently on
 * @return true if backlight > 0
 */
bool display_is_on();

/**
 * @brief Get display width in current rotation
 */
uint16_t display_get_width();

/**
 * @brief Get display height in current rotation
 */
uint16_t display_get_height();

} // namespace HAL
} // namespace CydOS
