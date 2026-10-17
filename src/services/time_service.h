/**
 * @file time_service.h
 * @brief System Time & Clock Service for CYD OS
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

namespace CydOS {
namespace Services {

/**
 * @brief Initialize system time service
 */
void time_init();

/**
 * @brief Periodic update (call in main loop to dispatch 1s ticks)
 */
void time_update();

/**
 * @brief Set time manually
 */
void time_set(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, uint16_t year);

/**
 * @brief Get formatted time string (e.g. "12:45" or "12:45:00")
 */
void time_get_formatted(char* buf, size_t maxLen, bool includeSeconds = false);

/**
 * @brief Get formatted date string (e.g. "Saturday, Oct 17")
 */
void time_get_date_formatted(char* buf, size_t maxLen);

/**
 * @brief Get current components
 */
uint8_t time_get_hour();
uint8_t time_get_minute();
uint8_t time_get_second();

} // namespace Services
} // namespace CydOS
