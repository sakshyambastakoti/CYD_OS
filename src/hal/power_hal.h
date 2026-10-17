/**
 * @file power_hal.h
 * @brief Power & Sleep Management HAL for CYD OS
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

namespace CydOS {
namespace HAL {

/**
 * @brief Initialize power management
 */
void power_init();

/**
 * @brief Poll power state & inactivity timer (call in loop)
 */
void power_update();

/**
 * @brief Reset user inactivity timer (e.g. on touch or UI action)
 */
void power_notify_activity();

/**
 * @brief Check if display is currently sleeping
 */
bool power_is_sleeping();

/**
 * @brief Wake the display immediately
 */
void power_wake();

/**
 * @brief Put display to sleep immediately
 */
void power_sleep();

/**
 * @brief Set display sleep timeout in seconds (0 = never sleep)
 */
void power_set_timeout(uint32_t seconds);

/**
 * @brief Get current sleep timeout in seconds
 */
uint32_t power_get_timeout();

/**
 * @brief Enable or disable ambient light auto-brightness
 */
void power_set_auto_brightness(bool enable);

/**
 * @brief Check if auto-brightness is enabled
 */
bool power_is_auto_brightness();

} // namespace HAL
} // namespace CydOS
