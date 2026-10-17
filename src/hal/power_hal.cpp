/**
 * @file power_hal.cpp
 * @brief Power & Sleep Management HAL implementation
 */

#include "power_hal.h"
#include "display_hal.h"
#include "adc_hal.h"
#include "touch_hal.h"
#include "app_config.h"
#include <Arduino.h>

namespace CydOS {
namespace HAL {

static uint32_t s_last_activity_time = 0;
static uint32_t s_timeout_seconds    = 30;
static bool     s_is_sleeping        = false;
static bool     s_auto_brightness    = false;
static uint8_t  s_active_brightness  = DEFAULT_BRIGHTNESS;

void power_init() {
    s_last_activity_time = millis();
    s_is_sleeping = false;
    Serial.printf("[POWER] Power manager initialized (Timeout: %u sec)\n", s_timeout_seconds);
}

void power_notify_activity() {
    s_last_activity_time = millis();
    if (s_is_sleeping) {
        power_wake();
    }
}

bool power_is_sleeping() {
    return s_is_sleeping;
}

void power_wake() {
    if (!s_is_sleeping) return;
    s_is_sleeping = false;
    s_last_activity_time = millis();
    display_set_brightness(s_active_brightness);
    Serial.println("[POWER] Display woke up");
}

void power_sleep() {
    if (s_is_sleeping) return;
    s_is_sleeping = true;
    display_set_brightness(0); // Turn off backlight
    Serial.println("[POWER] Display entered sleep");
}

void power_set_timeout(uint32_t seconds) {
    s_timeout_seconds = seconds;
    power_notify_activity();
}

uint32_t power_get_timeout() {
    return s_timeout_seconds;
}

void power_set_auto_brightness(bool enable) {
    s_auto_brightness = enable;
}

bool power_is_auto_brightness() {
    return s_auto_brightness;
}

void power_update() {
    uint32_t now = millis();

    // 1. Check for touch activity to wake or keep awake
    if (touch_is_pressed()) {
        power_notify_activity();
    }

    // 2. Inactivity sleep check
    if (!s_is_sleeping && s_timeout_seconds > 0) {
        if ((now - s_last_activity_time) >= (s_timeout_seconds * 1000)) {
            power_sleep();
        }
    }

    // 3. Auto-brightness adjustment (when awake)
    if (!s_is_sleeping && s_auto_brightness) {
        static uint32_t last_ldr_check = 0;
        if (now - last_ldr_check > 500) {
            last_ldr_check = now;
            uint8_t light_pct = adc_read_ldr_percent();
            // Scale light 0-100% to backlight 40-255
            uint8_t target_bl = 40 + (uint8_t)(((uint32_t)light_pct * 215) / 100);
            display_set_brightness(target_bl);
        }
    }
}

} // namespace HAL
} // namespace CydOS
