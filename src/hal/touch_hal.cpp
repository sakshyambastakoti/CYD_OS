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
#include "services/storage_service.h"
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
static int16_t  s_raw_x = 0;
static int16_t  s_raw_y = 0;
static bool     s_pressed = false;
static uint32_t s_last_touch_time = 0;
static bool     s_calibrating = false;

// ==========================================================================
// LVGL touch read callback
// ==========================================================================
static void lvgl_touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    uint16_t tx, ty;
    bool touched = g_display.getTouch(&tx, &ty);

    if (touched) {
        s_last_x = (int16_t)tx;
        s_last_y = (int16_t)ty;
        s_pressed = true;

        data->state = LV_INDEV_STATE_PR;
        data->point.x = s_last_x;
        data->point.y = s_last_y;

        // Diagnostic log throttled to ~5 Hz to avoid flooding Serial
        static uint32_t last_log = 0;
        if (millis() - last_log > 200) {
            last_log = millis();
            Serial.printf("[TOUCH] Screen: X=%d Y=%d\n", s_last_x, s_last_y);
        }
    } else {
        s_pressed = false;

        data->state = LV_INDEV_STATE_REL;
        // Keep last coordinates to allow LVGL click completion
        data->point.x = s_last_x;
        data->point.y = s_last_y;
    }
}

namespace CydOS {
namespace HAL {

bool touch_init() {
    // Check if custom user calibration is stored in NVS flash
    auto& settings = Services::storage_get_settings();
    if (settings.touch_calibrated) {
        bool valid = true;
        for (int i = 0; i < 8; i++) {
            if (settings.touch_cal[i] < 100 || settings.touch_cal[i] > 4000) {
                valid = false;
                break;
            }
        }
        if (valid) {
            Serial.println("[TOUCH] Applying saved calibration from NVS flash...");
            g_display.setTouchCalibrate(settings.touch_cal);
        } else {
            Serial.println("[TOUCH] Stored calibration was invalid, clearing.");
            settings.touch_calibrated = false;
            memset(settings.touch_cal, 0, sizeof(settings.touch_cal));
        }
    } else {
        Serial.println("[TOUCH] Using default hardware profile calibration");
    }

    lv_indev_drv_init(&s_indev_drv);
    s_indev_drv.type    = LV_INDEV_TYPE_POINTER;
    s_indev_drv.read_cb = lvgl_touch_read_cb;

    lv_indev_t *indev = lv_indev_drv_register(&s_indev_drv);
    if (indev == NULL) {
        Serial.println("[TOUCH] ERROR: LVGL input device registration failed!");
        return false;
    }

    Serial.println("[TOUCH] Input device registered (XPT2046)");
    return true;
}

int16_t touch_get_x() {
    return s_last_x;
}

int16_t touch_get_y() {
    return s_last_y;
}

bool touch_get_raw(int16_t* rx, int16_t* ry) {
    lgfx::touch_point_t tp;
    if (g_display.getTouchRaw(&tp)) {
        if (rx) *rx = tp.x;
        if (ry) *ry = tp.y;
        return true;
    }
    return false;
}

bool touch_is_pressed() {
    return s_pressed;
}

void touch_start_calibration() {
    s_calibrating = true;
    Serial.println("[TOUCH] Calibration mode started");
}

bool touch_is_calibrating() {
    return s_calibrating;
}

void touch_set_calibration(const uint16_t* cal) {
    if (cal) {
        uint16_t params[8];
        memcpy(params, cal, sizeof(params));
        g_display.setTouchCalibrate(params);
        Serial.printf("[TOUCH] Calibrated: [%u, %u, %u, %u, %u, %u, %u, %u]\n",
                      params[0], params[1], params[2], params[3],
                      params[4], params[5], params[6], params[7]);
    }
}

} // namespace HAL
} // namespace CydOS
