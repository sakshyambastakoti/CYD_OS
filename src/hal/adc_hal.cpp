/**
 * @file adc_hal.cpp
 * @brief ADC HAL implementation for CYD OS
 * 
 * GPIO 34: LDR (ambient light) — input only, no pull-up
 * GPIO 35: Battery voltage (optional) — input only, no pull-up
 * 
 * Both pins are on ADC1, which is safe to use alongside WiFi.
 * (ADC2 is unavailable when WiFi is active on ESP32.)
 */

#include "adc_hal.h"
#include "pin_config.h"
#include <Arduino.h>

// ==========================================================================
// Module state
// ==========================================================================
static const int ADC_SAMPLES = 8;  // Average N samples for noise reduction

// Read N samples and return average
static uint16_t adc_read_averaged(uint8_t pin) {
    uint32_t sum = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) {
        sum += analogRead(pin);
        delayMicroseconds(100);
    }
    return (uint16_t)(sum / ADC_SAMPLES);
}

namespace CydOS {
namespace HAL {

void adc_init() {
    // Configure ADC resolution (12-bit default on ESP32)
    analogReadResolution(12);

    // Set attenuation for full 3.3V range
    analogSetPinAttenuation(PIN_LDR, ADC_11db);
    analogSetPinAttenuation(PIN_BATTERY_ADC, ADC_11db);

    Serial.println("[ADC] ADC initialized (12-bit, 11dB attenuation)");

    // Initial readings
    uint16_t ldr = adc_read_ldr();
    uint16_t bat = adc_read_battery_raw();
    Serial.printf("[ADC] LDR: %d, Battery ADC: %d\n", ldr, bat);
}

uint16_t adc_read_ldr() {
    return adc_read_averaged(PIN_LDR);
}

uint8_t adc_get_light_percent() {
    uint16_t raw = adc_read_ldr();
    // LDR: higher value = more light (may need inversion depending on circuit)
    // Map 0-4095 to 0-100
    return (uint8_t)map(raw, 0, 4095, 0, 100);
}

uint16_t adc_read_battery_raw() {
    return adc_read_averaged(PIN_BATTERY_ADC);
}

bool adc_battery_circuit_detected() {
    // Read multiple times to check if pin is connected vs floating
    uint16_t readings[4];
    for (int i = 0; i < 4; i++) {
        readings[i] = analogRead(PIN_BATTERY_ADC);
        delay(10);
    }

    // Check variance — floating pin has high variance
    uint16_t minVal = readings[0], maxVal = readings[0];
    for (int i = 1; i < 4; i++) {
        if (readings[i] < minVal) minVal = readings[i];
        if (readings[i] > maxVal) maxVal = readings[i];
    }

    uint16_t variance = maxVal - minVal;

    // If readings are stable (low variance) and non-zero, circuit likely present
    // This is a heuristic — a floating input-only pin with no pull-up
    // will read unpredictable values
    bool detected = (variance < 200) && (readings[0] > 100);

    Serial.printf("[ADC] Battery circuit detection: variance=%d, val=%d, detected=%s\n",
                  variance, readings[0], detected ? "YES" : "NO");

    return detected;
}

} // namespace HAL
} // namespace CydOS
