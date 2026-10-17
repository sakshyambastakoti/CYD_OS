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
    if (m_lbl_step) {
        lv_label_set_text(m_lbl_step, "Calibration Complete!\nSaved to Flash.");
    }
    if (m_target) {
        lv_obj_add_flag(m_target, LV_OBJ_FLAG_HIDDEN);
    }

    Services::storage_get_settings().touch_calibrated = true;
    Services::storage_save();

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
        m_step++;

        if (m_step == 1) {
            lv_label_set_text(m_lbl_step, "Touch Top-Right Target (2/4)");
            set_target_pos(215, 20);
        } else if (m_step == 2) {
            lv_label_set_text(m_lbl_step, "Touch Bottom-Right Target (3/4)");
            set_target_pos(215, 250);
        } else if (m_step == 3) {
            lv_label_set_text(m_lbl_step, "Touch Bottom-Left Target (4/4)");
            set_target_pos(25, 250);
        } else if (m_step >= 4) {
            finish_calibration();
        }
    }
}

lv_obj_t* TouchCalApp::create_screen() {
    m_screen = lv_obj_create(nullptr);
    lv_obj_add_style(m_screen, &UI::style_screen_bg, 0);
    lv_obj_set_size(m_screen, 240, 274);
    lv_obj_set_pos(m_screen, 0, 22);
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
    lv_obj_set_style_bg_color(m_target, UI::THEME_COLOR_PRIMARY, 0);
    lv_obj_set_style_bg_opa(m_target, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(m_target, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(m_target, 2, 0);
    lv_obj_set_style_border_color(m_target, lv_color_white(), 0);

    set_target_pos(25, 20);

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
