/**
 * @file touch_hal.cpp
 * @brief Touch HAL implementation for CYD OS
 * 
 * Reads XPT2046 via LovyanGFX's built-in touch support.
 * Implements debounce, coordinate caching, and LVGL input driver.
 */

#include "touch_hal.h"
#include "cyd_config.h"
#include "app_config.h"
#include <lvgl.h>

// ==========================================================================
// External display instance (defined in display_hal.cpp)
// ==========================================================================
extern LGFX_CYD g_display;

// ==========================================================================
// Module state
// ==========================================================================
static lv_indev_drv_t s_indev_drv;
static int16_t  s_last_x = 0;
static int16_t  s_last_y = 0;
static bool     s_pressed = false;
static uint32_t s_last_touch_time = 0;
static bool     s_calibrating = false;

// ==========================================================================
// LVGL touch read callback
// ==========================================================================
static void lvgl_touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    uint16_t tx, ty;
    bool touched = g_display.getTouch(&tx, &ty);

    uint32_t now = millis();

    if (touched) {
        // Debounce: only accept new touch if enough time since last release
        if (!s_pressed && (now - s_last_touch_time) < TOUCH_DEBOUNCE_MS) {
            data->state = LV_INDEV_STATE_REL;
            return;
        }

        // Cache coordinates (prevent false 0,0 on release)
        s_last_x = (int16_t)tx;
        s_last_y = (int16_t)ty;
        s_pressed = true;

        data->state = LV_INDEV_STATE_PR;
        data->point.x = s_last_x;
        data->point.y = s_last_y;
    } else {
        if (s_pressed) {
            s_last_touch_time = now;  // Record release time for debounce
        }
        s_pressed = false;

        data->state = LV_INDEV_STATE_REL;
        // Return last known coordinates (not 0,0)
        data->point.x = s_last_x;
        data->point.y = s_last_y;
    }
}

namespace CydOS {
namespace HAL {

bool touch_init() {
    // Touch hardware is configured via LovyanGFX in cyd_config.h
    // We just need to register the LVGL input device

    lv_indev_drv_init(&s_indev_drv);
    s_indev_drv.type    = LV_INDEV_TYPE_POINTER;
    s_indev_drv.read_cb = lvgl_touch_read_cb;

    lv_indev_t *indev = lv_indev_drv_register(&s_indev_drv);
    if (indev == NULL) {
        Serial.println("[TOUCH] ERROR: LVGL input device registration failed!");
        return false;
    }

    Serial.println("[TOUCH] Input device registered (XPT2046, polled)");
    return true;
}

int16_t touch_get_x() {
    return s_last_x;
}

int16_t touch_get_y() {
    return s_last_y;
}

bool touch_is_pressed() {
    uint16_t x, y;
    if (g_display.getTouch(&x, &y)) {
        s_last_x = (int16_t)x;
        s_last_y = (int16_t)y;
        return true;
    }
    return s_pressed;
}

void touch_start_calibration() {
    s_calibrating = true;
    Serial.println("[TOUCH] Calibration mode started");
}

bool touch_is_calibrating() {
    return s_calibrating;
}

} // namespace HAL
} // namespace CydOS
