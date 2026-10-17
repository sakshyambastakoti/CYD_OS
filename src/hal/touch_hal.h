/**
 * @file touch_hal.h
 * @brief Touch Input Hardware Abstraction Layer for CYD OS
 * 
 * Handles XPT2046 resistive touch reading with debounce,
 * coordinate filtering, and LVGL input device registration.
 */

#pragma once

#include <lvgl.h>

namespace CydOS {
namespace HAL {

/**
 * @brief Initialize the touch controller and register LVGL input device
 * @return true if touch initialized successfully
 */
bool touch_init();

/**
 * @brief Get last valid touch X coordinate
 */
int16_t touch_get_x();

/**
 * @brief Get last valid touch Y coordinate
 */
int16_t touch_get_y();

/**
 * @brief Check if screen is currently being touched
 */
bool touch_is_pressed();

/**
 * @brief Enable/disable touch calibration mode
 */
void touch_start_calibration();

/**
 * @brief Check if touch calibration is in progress
 */
bool touch_is_calibrating();

/**
 * @brief Get raw ADC reading from touch controller
 */
bool touch_get_raw(int16_t* rx, int16_t* ry);

/**
 * @brief Apply 8-point affine calibration parameters to LovyanGFX
 */
void touch_set_calibration(const uint16_t* cal);

} // namespace HAL
} // namespace CydOS
