/**
 * @file adc_hal.h
 * @brief ADC Hardware Abstraction Layer for CYD OS
 * 
 * Reads the onboard LDR (ambient light sensor) on GPIO 34
 * and optional battery voltage on GPIO 35 (external circuit required).
 */

#pragma once

#include <stdint.h>

namespace CydOS {
namespace HAL {

/**
 * @brief Initialize ADC channels
 */
void adc_init();

/**
 * @brief Read ambient light level from LDR
 * @return Raw ADC value (0-4095, 12-bit)
 */
uint16_t adc_read_ldr();

/**
 * @brief Get ambient light as percentage (0=dark, 100=bright)
 * @return Light percentage
 */
uint8_t adc_get_light_percent();

/**
 * @brief Read battery ADC (GPIO 35)
 * @return Raw ADC value (0-4095), or 0 if no battery circuit
 * 
 * @note Battery ADC requires an external voltage divider circuit.
 *       Do NOT connect a battery directly to GPIO.
 *       Without verified external circuit, this returns raw ADC only.
 */
uint16_t adc_read_battery_raw();

/**
 * @brief Check if battery monitoring circuit is detected
 * @return true if battery ADC reads non-zero and appears connected
 * 
 * @note This is a heuristic, not definitive hardware detection.
 *       A floating pin may read random values.
 */
bool adc_battery_circuit_detected();

} // namespace HAL
} // namespace CydOS
