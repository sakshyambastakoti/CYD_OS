/**
 * @file app_touch_cal.cpp
 * @brief Interactive Touch Calibration Utility implementation
 */

#include "app_touch_cal.h"
#include "ui/theme.h"
#include "ui/app_manager.h"
#include "hal/touch_hal.h"
#include "hal/audio_hal.h"
#include "services/storage_service.h"
#include <Arduino.h>

namespace CydOS {
namespace Apps {

TouchCalApp::TouchCalApp()
    : m_screen(nullptr), m_target(nullptr), m_lbl_step(nullptr),
      m_step(0), m_last_step_time(0), m_waiting_release(false) {
    memset(m_samples, 0, sizeof(m_samples));
}

TouchCalApp::~TouchCalApp() {
    destroy_screen();
}

void TouchCalApp::set_target_pos(int16_t x, int16_t y) {
    if (m_target) {
        lv_obj_set_pos(m_target, x - 12, y - 12);
    }
}

void TouchCalApp::finish_calibration() {
    HAL::audio_play_notification();

    // Extrapolate the 4 corners of the 240x320 screen from the 4 sampled points:
    // Sample 0: TL at (25, 45)  -> (rx0, ry0)
    // Sample 1: TR at (215, 45) -> (rx1, ry1)
    // Sample 2: BR at (215, 275)-> (rx2, ry2)
    // Sample 3: BL at (25, 275) -> (rx3, ry3)
    int32_t rx0 = m_samples[0][0], ry0 = m_samples[0][1];
    int32_t rx1 = m_samples[1][0], ry1 = m_samples[1][1];
    int32_t rx2 = m_samples[2][0], ry2 = m_samples[2][1];
    int32_t rx3 = m_samples[3][0], ry3 = m_samples[3][1];

    uint16_t cal[8];
    // Corner 0: (0, 0) [Top-Left]
    cal[0] = (uint16_t)constrain(rx0 - (rx1 - rx0) * 25 / 190, 50, 4000);
    cal[1] = (uint16_t)constrain(ry0 - (ry3 - ry0) * 45 / 230, 50, 4000);

    // Corner 1: (0, 319) [Bottom-Left]
    cal[2] = (uint16_t)constrain(rx3 - (rx2 - rx3) * 25 / 190, 50, 4000);
    cal[3] = (uint16_t)constrain(ry3 + (ry3 - ry0) * 44 / 230, 50, 4000);

    // Corner 2: (239, 0) [Top-Right]
    cal[4] = (uint16_t)constrain(rx1 + (rx1 - rx0) * 24 / 190, 50, 4000);
    cal[5] = (uint16_t)constrain(ry1 - (ry2 - ry1) * 45 / 230, 50, 4000);

    // Corner 3: (239, 319) [Bottom-Right]
    cal[6] = (uint16_t)constrain(rx2 + (rx2 - rx3) * 24 / 190, 50, 4000);
    cal[7] = (uint16_t)constrain(ry2 + (ry2 - ry1) * 44 / 230, 50, 4000);

    HAL::touch_set_calibration(cal);

    auto& settings = Services::storage_get_settings();
    settings.touch_calibrated = true;
    memcpy(settings.touch_cal, cal, sizeof(cal));
    Services::storage_save();

    if (m_lbl_step) {
        lv_label_set_text(m_lbl_step, "Calibration Complete!\nSaved to Flash.");
    }
    if (m_target) {
        lv_obj_add_flag(m_target, LV_OBJ_FLAG_HIDDEN);
    }

    delay(800);
    UI::app_manager_pop();
}

void TouchCalApp::update() {
    if (m_screen == nullptr || m_step >= 4) return;

    bool pressed = HAL::touch_is_pressed();

    if (m_waiting_release) {
        if (!pressed) {
            m_waiting_release = false;
        }
        return;
    }

    if (pressed) {
        HAL::audio_play_click();
        m_waiting_release = true;

        int16_t rx = 0, ry = 0;
        HAL::touch_get_raw(&rx, &ry);
        if (m_step < 4) {
            m_samples[m_step][0] = rx;
            m_samples[m_step][1] = ry;
            Serial.printf("[TOUCHCAL] Sample %u: Raw=(%d, %d)\n", m_step, rx, ry);
        }

        m_step++;

        if (m_step == 1) {
            lv_label_set_text(m_lbl_step, "Touch Top-Right Target (2/4)");
            set_target_pos(215, 45);
        } else if (m_step == 2) {
            lv_label_set_text(m_lbl_step, "Touch Bottom-Right Target (3/4)");
            set_target_pos(215, 275);
        } else if (m_step == 3) {
            lv_label_set_text(m_lbl_step, "Touch Bottom-Left Target (4/4)");
            set_target_pos(25, 275);
        } else if (m_step >= 4) {
            finish_calibration();
        }
    }
}

lv_obj_t* TouchCalApp::create_screen() {
    m_screen = lv_obj_create(nullptr);
    lv_obj_add_style(m_screen, &UI::style_screen_bg, 0);
    lv_obj_set_size(m_screen, 240, 320);
    lv_obj_set_pos(m_screen, 0, 0);
    lv_obj_clear_flag(m_screen, LV_OBJ_FLAG_SCROLLABLE);

    m_step = 0;
    m_waiting_release = true;

    // Step instruction label
    m_lbl_step = lv_label_create(m_screen);
    lv_obj_add_style(m_lbl_step, &UI::style_title, 0);
    lv_label_set_text(m_lbl_step, "Touch Top-Left Target (1/4)");
    lv_obj_align(m_lbl_step, LV_ALIGN_CENTER, 0, -30);
    lv_obj_set_style_text_align(m_lbl_step, LV_TEXT_ALIGN_CENTER, 0);

    // Crosshair Target indicator (24x24 neon circle with center dot)
    m_target = lv_obj_create(m_screen);
    lv_obj_remove_style_all(m_target);
    lv_obj_set_size(m_target, 24, 24);
    lv_obj_set_style_bg_color(m_target, THEME_COLOR_PRIMARY, 0);
    lv_obj_set_style_bg_opa(m_target, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(m_target, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(m_target, 2, 0);
    lv_obj_set_style_border_color(m_target, lv_color_white(), 0);

    set_target_pos(25, 45);

    return m_screen;
}

void TouchCalApp::destroy_screen() {
    if (m_screen) {
        lv_obj_del(m_screen);
        m_screen = nullptr;
        m_target = nullptr;
        m_lbl_step = nullptr;
    }
}

} // namespace Apps
} // namespace CydOS
