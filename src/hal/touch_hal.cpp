/**
 * @file touch_hal.cpp
 * @brief Industrial-Grade Touch HAL implementation for CYD OS
 * 
 * Target: ESP32-2432S028R (ST7789 + XPT2046 Resistive Touch)
 * Features:
 *  - Exponential Moving Average (EMA) low-pass noise filter
 *  - Tap Anchor Lock: prevents button misfires caused by resistive lift-off drift
 *  - Two-stage temporal debounce on both press and release
 *  - Coordinated calibration exclusion guard
 *  - Fallback to verified factory calibration matrix
 */

#include "touch_hal.h"
#include "cyd_config.h"
#include "app_config.h"
#include "services/storage_service.h"
#include <lvgl.h>
#include <Arduino.h>

// ==========================================================================
// External display instance (defined in display_hal.cpp)
// ==========================================================================
extern LGFX_CYD g_display;

// ==========================================================================
// Filter Constants & Parameters
// ==========================================================================
// Maximum squared movement (pixels^2) allowed before a tap becomes a drag.
// 14 pixels corresponds to standard finger contact deformation radius.
#define TOUCH_TAP_MAX_JITTER_SQ     (14 * 14)

// EMA smoothing weights: 0.55 previous + 0.45 current raw
#define TOUCH_EMA_WEIGHT_PREV       0.55f
#define TOUCH_EMA_WEIGHT_RAW        0.45f

// Consecutive read debouncing
#define TOUCH_DEBOUNCE_PRESS_CNT    2   // 2 cycles (~15-20 ms) to confirm press
#define TOUCH_DEBOUNCE_REL_CNT      2   // 2 cycles (~15-20 ms) to confirm release

// Default verified factory calibration matrix:
// [TL_X, TL_Y, BL_X, BL_Y, TR_X, TR_Y, BR_X, BR_Y]
static const uint16_t s_factory_cal[8] = { 200, 3800, 200, 240, 3850, 3800, 3850, 240 };

// ==========================================================================
// Module State Machine
// ==========================================================================
static lv_indev_drv_t s_indev_drv;

static int16_t  s_reported_x = 0;
static int16_t  s_reported_y = 0;

static float    s_filtered_x = 0.0f;
static float    s_filtered_y = 0.0f;

static int16_t  s_anchor_x = 0;
static int16_t  s_anchor_y = 0;

static int16_t  s_penultimate_x = 0;
static int16_t  s_penultimate_y = 0;

static bool     s_is_pressed = false;
static bool     s_is_drag = false;
static uint8_t  s_press_consecutive = 0;
static uint8_t  s_release_consecutive = 0;

static bool     s_calibrating = false;

// ==========================================================================
// LVGL Touch Read Callback (Industrial State Machine)
// ==========================================================================
static void lvgl_touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    // 1. If calibration utility is currently running, yield immediately to avoid SPI contention
    if (s_calibrating) {
        data->state = LV_INDEV_STATE_REL;
        data->point.x = s_reported_x;
        data->point.y = s_reported_y;
        return;
    }

    uint16_t raw_x = 0, raw_y = 0;
    bool hardware_touched = g_display.getTouch(&raw_x, &raw_y);

    // Validate raw coordinate bounds [0..239, 0..319]
    bool coordinate_valid = hardware_touched && (raw_x < DISPLAY_WIDTH) && (raw_y < DISPLAY_HEIGHT);

    if (coordinate_valid) {
        s_release_consecutive = 0;
        if (s_press_consecutive < 255) s_press_consecutive++;

        if (!s_is_pressed) {
            // Check debounce threshold before acknowledging touch down
            if (s_press_consecutive >= TOUCH_DEBOUNCE_PRESS_CNT) {
                s_is_pressed = true;
                s_is_drag = false;

                s_filtered_x = (float)raw_x;
                s_filtered_y = (float)raw_y;

                s_anchor_x = (int16_t)raw_x;
                s_anchor_y = (int16_t)raw_y;

                s_penultimate_x = s_anchor_x;
                s_penultimate_y = s_anchor_y;

                s_reported_x = s_anchor_x;
                s_reported_y = s_anchor_y;

                data->state = LV_INDEV_STATE_PR;
                data->point.x = s_reported_x;
                data->point.y = s_reported_y;

                Serial.printf("[TOUCH] PRESS @ (%d, %d)\n", s_reported_x, s_reported_y);
            } else {
                // In debounce holding phase
                data->state = LV_INDEV_STATE_REL;
                data->point.x = s_reported_x;
                data->point.y = s_reported_y;
            }
        } else {
            // Touch is actively held down
            s_penultimate_x = (int16_t)s_filtered_x;
            s_penultimate_y = (int16_t)s_filtered_y;

            // Apply Exponential Moving Average (EMA) smoothing
            s_filtered_x = (s_filtered_x * TOUCH_EMA_WEIGHT_PREV) + ((float)raw_x * TOUCH_EMA_WEIGHT_RAW);
            s_filtered_y = (s_filtered_y * TOUCH_EMA_WEIGHT_PREV) + ((float)raw_y * TOUCH_EMA_WEIGHT_RAW);

            int32_t dx = (int32_t)s_filtered_x - (int32_t)s_anchor_x;
            int32_t dy = (int32_t)s_filtered_y - (int32_t)s_anchor_y;
            int32_t dist_sq = dx * dx + dy * dy;

            if (!s_is_drag) {
                if (dist_sq > TOUCH_TAP_MAX_JITTER_SQ) {
                    // Finger moved beyond jitter radius — user is intentionally scrolling/dragging
                    s_is_drag = true;
                    s_reported_x = (int16_t)s_filtered_x;
                    s_reported_y = (int16_t)s_filtered_y;
                } else {
                    // Tap Anchor Lock: keep reporting origin to prevent lift-off misclicks!
                    s_reported_x = s_anchor_x;
                    s_reported_y = s_anchor_y;
                }
            } else {
                s_reported_x = (int16_t)s_filtered_x;
                s_reported_y = (int16_t)s_filtered_y;
            }

            data->state = LV_INDEV_STATE_PR;
            data->point.x = s_reported_x;
            data->point.y = s_reported_y;
        }
    } else {
        // Hardware reports un-touched
        s_press_consecutive = 0;
        if (s_release_consecutive < 255) s_release_consecutive++;

        if (s_is_pressed) {
            if (s_release_consecutive >= TOUCH_DEBOUNCE_REL_CNT) {
                s_is_pressed = false;

                // For a tap, lock release point to anchor to guarantee button click completion
                if (!s_is_drag) {
                    s_reported_x = s_anchor_x;
                    s_reported_y = s_anchor_y;
                    Serial.printf("[TOUCH] TAP RELEASE @ (%d, %d)\n", s_reported_x, s_reported_y);
                } else {
                    // For a drag, report the penultimate stable point (avoids lift-off bounce)
                    s_reported_x = s_penultimate_x;
                    s_reported_y = s_penultimate_y;
                    Serial.printf("[TOUCH] DRAG RELEASE @ (%d, %d)\n", s_reported_x, s_reported_y);
                }

                data->state = LV_INDEV_STATE_REL;
                data->point.x = s_reported_x;
                data->point.y = s_reported_y;
            } else {
                // Brief single-sample gap, maintain pressed state to prevent chatter
                data->state = LV_INDEV_STATE_PR;
                data->point.x = s_reported_x;
                data->point.y = s_reported_y;
            }
        } else {
            data->state = LV_INDEV_STATE_REL;
            data->point.x = s_reported_x;
            data->point.y = s_reported_y;
        }
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
            Serial.println("[TOUCH] Stored calibration was invalid, falling back to factory defaults.");
            settings.touch_calibrated = false;
            memset(settings.touch_cal, 0, sizeof(settings.touch_cal));
            touch_set_calibration(s_factory_cal);
        }
    } else {
        Serial.println("[TOUCH] Applying verified factory default calibration profile");
        touch_set_calibration(s_factory_cal);
    }

    lv_indev_drv_init(&s_indev_drv);
    s_indev_drv.type            = LV_INDEV_TYPE_POINTER;
    s_indev_drv.read_cb         = lvgl_touch_read_cb;
    s_indev_drv.scroll_limit    = 14;     // Jitter threshold matching tap anchor radius
    s_indev_drv.long_press_time = 500;    // Standard long press duration in ms

    lv_indev_t *indev = lv_indev_drv_register(&s_indev_drv);
    if (indev == NULL) {
        Serial.println("[TOUCH] ERROR: LVGL input device registration failed!");
        return false;
    }

    Serial.println("[TOUCH] Input device registered (XPT2046 with Tap Anchor & Debounce)");
    return true;
}

int16_t touch_get_x() {
    return s_reported_x;
}

int16_t touch_get_y() {
    return s_reported_y;
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
    return s_is_pressed;
}

void touch_start_calibration() {
    s_calibrating = true;
    Serial.println("[TOUCH] Calibration mode started: LVGL input suspended");
}

void touch_finish_calibration() {
    s_calibrating = false;
    s_is_pressed = false;
    s_press_consecutive = 0;
    s_release_consecutive = TOUCH_DEBOUNCE_REL_CNT;
    Serial.println("[TOUCH] Calibration mode ended: LVGL input resumed");
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
