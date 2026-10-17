/**
 * @file led_hal.h
 * @brief RGB LED Hardware Abstraction Layer for CYD OS
 * 
 * Controls the onboard RGB LED (Active LOW — common anode).
 * GPIO 4 = Red, GPIO 16 = Green, GPIO 17 = Blue.
 */

#pragma once

#include <stdint.h>

namespace CydOS {
namespace HAL {

/**
 * @brief Initialize RGB LED pins as outputs (all OFF)
 */
void led_init();

/**
 * @brief Set LED color using individual R/G/B booleans
 * @param red   true = red ON
 * @param green true = green ON
 * @param blue  true = blue ON
 */
void led_set(bool red, bool green, bool blue);

/**
 * @brief Set LED to a specific named color
 */
void led_off();
void led_red();
void led_green();
void led_blue();
void led_cyan();
void led_magenta();
void led_yellow();
void led_white();

/**
 * @brief Flash the LED a given color for specified duration
 * @param red   Red channel
 * @param green Green channel
 * @param blue  Blue channel
 * @param duration_ms Flash duration in milliseconds
 */
void led_flash(bool red, bool green, bool blue, uint32_t duration_ms);

} // namespace HAL
} // namespace CydOS
