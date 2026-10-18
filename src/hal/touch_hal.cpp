/**
 * @file touch_hal.cpp
 * @brief Clean, High-Performance Touch HAL for CYD OS
 * 
 * Target: ESP32-2432S028R (ST7789 + XPT2046 Resistive Touch)
 * Uses native LovyanGFX touch processing directly without artificial debounce delays.
 */

#include "touch_hal.h"
#include "cyd_config.h"
#include "app_config.h"
#include "services/storage_service.h"
#include <lvgl.h>
#include <Arduino.h>

extern LGFX_CYD g_display;

static lv_indev_drv_t s_indev_drv;
static int16_t  s_last_x = 0;
static int16_t  s_last_y = 0;
static bool     s_is_pressed = false;
static bool     s_calibrating = false;

static void lvgl_touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    if (s_calibrating) {
        data->state = LV_INDEV_STATE_REL;
        data->point.x = s_last_x;
        data->point.y = s_last_y;
        return;
    }

    uint16_t tx = 0, ty = 0;
    bool touched = g_display.getTouch(&tx, &ty);

    if (touched) {
        // Clamp coordinates within panel dimensions
        if (tx >= DISPLAY_WIDTH)  tx = DISPLAY_WIDTH - 1;
        if (ty >= DISPLAY_HEIGHT) ty = DISPLAY_HEIGHT - 1;

        s_last_x = (int16_t)tx;
        s_last_y = (int16_t)ty;
        s_is_pressed = true;

        data->state = LV_INDEV_STATE_PR;
        data->point.x = s_last_x;
        data->point.y = s_last_y;

        static uint32_t last_log = 0;
        if (millis() - last_log > 250) {
            last_log = millis();
            Serial.printf("[TOUCH] Pressed @ (%d, %d)\n", s_last_x, s_last_y);
        }
    } else {
        s_is_pressed = false;

        data->state = LV_INDEV_STATE_REL;
        data->point.x = s_last_x;
        data->point.y = s_last_y;
    }
}

namespace CydOS {
namespace HAL {

bool touch_init() {
    Serial.println("[TOUCH] Initializing XPT2046 with native LovyanGFX calibration...");

    // Only apply custom calibration if user explicitly calibrated via TouchCalApp
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
            Serial.println("[TOUCH] Applying user calibration from Flash...");
            g_display.setTouchCalibrate(settings.touch_cal);
        } else {
            settings.touch_calibrated = false;
        }
    }

    lv_indev_drv_init(&s_indev_drv);
    s_indev_drv.type            = LV_INDEV_TYPE_POINTER;
    s_indev_drv.read_cb         = lvgl_touch_read_cb;
    s_indev_drv.scroll_limit    = 8;
    s_indev_drv.long_press_time = 500;

    lv_indev_t *indev = lv_indev_drv_register(&s_indev_drv);
    if (indev == NULL) {
        Serial.println("[TOUCH] ERROR: LVGL input device registration failed!");
        return false;
    }

    Serial.println("[TOUCH] Input device registered successfully");
    return true;
}

int16_t touch_get_x() { return s_last_x; }
int16_t touch_get_y() { return s_last_y; }
bool touch_is_pressed() { return s_is_pressed; }

bool touch_get_raw(int16_t* rx, int16_t* ry) {
    lgfx::touch_point_t tp;
    if (g_display.getTouchRaw(&tp)) {
        if (rx) *rx = tp.x;
        if (ry) *ry = tp.y;
        return true;
    }
    return false;
}

void touch_start_calibration() {
    s_calibrating = true;
    Serial.println("[TOUCH] Calibration started: LVGL input suspended");
}

void touch_finish_calibration() {
    s_calibrating = false;
    s_is_pressed = false;
    Serial.println("[TOUCH] Calibration finished: LVGL input resumed");
}

bool touch_is_calibrating() { return s_calibrating; }

void touch_set_calibration(const uint16_t* cal) {
    if (cal) {
        uint16_t params[8];
        memcpy(params, cal, sizeof(params));
        g_display.setTouchCalibrate(params);
    }
}

} // namespace HAL
} // namespace CydOS
