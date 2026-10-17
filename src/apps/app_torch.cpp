/**
 * @file app_torch.cpp
 * @brief Torch & Flashlight Application implementation
 */

#include "app_torch.h"
#include "ui/theme.h"
#include "hal/display_hal.h"
#include "hal/led_hal.h"
#include "hal/audio_hal.h"
#include "services/storage_service.h"
#include <Arduino.h>

namespace CydOS {
namespace Apps {

TorchApp::TorchApp()
    : m_screen(nullptr), m_btn_screen_torch(nullptr), m_btn_led_torch(nullptr), m_btn_sos(nullptr),
      m_screen_torch_on(false), m_led_torch_on(false), m_sos_on(false),
      m_orig_brightness(200), m_sos_last_toggle(0), m_sos_state(false) {
}

TorchApp::~TorchApp() {
    destroy_screen();
}

void TorchApp::screen_torch_cb(lv_event_t* e) {
    HAL::audio_play_click();
    TorchApp* app = (TorchApp*)lv_event_get_user_data(e);
    app->m_screen_torch_on = !app->m_screen_torch_on;

    if (app->m_screen_torch_on) {
        app->m_orig_brightness = Services::storage_get_settings().brightness;
        HAL::display_set_brightness(255);
        lv_obj_set_style_bg_color(app->m_screen, lv_color_white(), 0);
        lv_obj_set_style_bg_color(app->m_btn_screen_torch, THEME_COLOR_PRIMARY, 0);
    } else {
        HAL::display_set_brightness(app->m_orig_brightness);
        lv_obj_set_style_bg_color(app->m_screen, THEME_COLOR_BG, 0);
        lv_obj_set_style_bg_color(app->m_btn_screen_torch, THEME_COLOR_SURFACE_ALT, 0);
    }
}

void TorchApp::led_torch_cb(lv_event_t* e) {
    HAL::audio_play_click();
    TorchApp* app = (TorchApp*)lv_event_get_user_data(e);
    app->m_sos_on = false; // Turn off SOS if toggling regular
    app->m_led_torch_on = !app->m_led_torch_on;

    if (app->m_led_torch_on) {
        HAL::led_set_rgb(true, true, true);
        lv_obj_set_style_bg_color(app->m_btn_led_torch, THEME_COLOR_PRIMARY, 0);
    } else {
        HAL::led_set_rgb(false, false, false);
        lv_obj_set_style_bg_color(app->m_btn_led_torch, THEME_COLOR_SURFACE_ALT, 0);
    }
}

void TorchApp::sos_cb(lv_event_t* e) {
    HAL::audio_play_click();
    TorchApp* app = (TorchApp*)lv_event_get_user_data(e);
    app->m_sos_on = !app->m_sos_on;
    app->m_led_torch_on = false;

    if (app->m_sos_on) {
        lv_obj_set_style_bg_color(app->m_btn_sos, THEME_COLOR_DANGER, 0);
    } else {
        HAL::led_set_rgb(false, false, false);
        lv_obj_set_style_bg_color(app->m_btn_sos, THEME_COLOR_SURFACE_ALT, 0);
    }
}

void TorchApp::update() {
    if (m_sos_on) {
        uint32_t now = millis();
        if (now - m_sos_last_toggle > 250) {
            m_sos_last_toggle = now;
            m_sos_state = !m_sos_state;
            HAL::led_set_rgb(m_sos_state, false, false); // Blinking red
        }
    }
}

lv_obj_t* TorchApp::create_screen() {
    m_screen = lv_obj_create(nullptr);
    lv_obj_add_style(m_screen, &UI::style_screen_bg, 0);
    lv_obj_set_size(m_screen, 240, 320);
    lv_obj_set_pos(m_screen, 0, 0);
    lv_obj_clear_flag(m_screen, LV_OBJ_FLAG_SCROLLABLE);

    // Title Card (below status bar)
    lv_obj_t* card = lv_obj_create(m_screen);
    lv_obj_add_style(card, &UI::style_card, 0);
    lv_obj_set_size(card, 224, 60);
    lv_obj_set_pos(card, 8, 26);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lbl_t = lv_label_create(card);
    lv_label_set_text(lbl_t, "Flashlight & Torch");
    lv_obj_add_style(lbl_t, &UI::style_title, 0);
    lv_obj_set_pos(lbl_t, 4, 4);

    lv_obj_t* lbl_sub = lv_label_create(card);
    lv_label_set_text(lbl_sub, "Screen & RGB hardware torch");
    lv_obj_add_style(lbl_sub, &UI::style_subtitle, 0);
    lv_obj_set_pos(lbl_sub, 4, 26);

    // 1. Screen Torch Button
    m_btn_screen_torch = lv_btn_create(m_screen);
    lv_obj_add_style(m_btn_screen_torch, &UI::style_btn, 0);
    lv_obj_set_size(m_btn_screen_torch, 224, 46);
    lv_obj_set_pos(m_btn_screen_torch, 8, 96);
    lv_obj_add_event_cb(m_btn_screen_torch, screen_torch_cb, LV_EVENT_CLICKED, this);
    lv_obj_t* lbl_s = lv_label_create(m_btn_screen_torch);
    lv_label_set_text(lbl_s, LV_SYMBOL_EYE_OPEN " Full Screen White Light");
    lv_obj_add_style(lbl_s, &UI::style_subtitle, 0);
    lv_obj_center(lbl_s);

    // 2. RGB LED Torch Button
    m_btn_led_torch = lv_btn_create(m_screen);
    lv_obj_add_style(m_btn_led_torch, &UI::style_btn, 0);
    lv_obj_set_size(m_btn_led_torch, 224, 46);
    lv_obj_set_pos(m_btn_led_torch, 8, 150);
    lv_obj_add_event_cb(m_btn_led_torch, led_torch_cb, LV_EVENT_CLICKED, this);
    lv_obj_t* lbl_l = lv_label_create(m_btn_led_torch);
    lv_label_set_text(lbl_l, LV_SYMBOL_IMAGE " Board RGB Torch");
    lv_obj_add_style(lbl_l, &UI::style_subtitle, 0);
    lv_obj_center(lbl_l);

    // 3. SOS Strobe Button
    m_btn_sos = lv_btn_create(m_screen);
    lv_obj_add_style(m_btn_sos, &UI::style_btn, 0);
    lv_obj_set_size(m_btn_sos, 224, 46);
    lv_obj_set_pos(m_btn_sos, 8, 204);
    lv_obj_add_event_cb(m_btn_sos, sos_cb, LV_EVENT_CLICKED, this);
    lv_obj_t* lbl_sos = lv_label_create(m_btn_sos);
    lv_label_set_text(lbl_sos, LV_SYMBOL_WARNING " Emergency SOS Strobe");
    lv_obj_add_style(lbl_sos, &UI::style_subtitle, 0);
    lv_obj_center(lbl_sos);

    return m_screen;
}

void TorchApp::destroy_screen() {
    if (m_screen_torch_on) {
        HAL::display_set_brightness(m_orig_brightness);
        m_screen_torch_on = false;
    }
    if (m_led_torch_on || m_sos_on) {
        HAL::led_set_rgb(false, false, false);
        m_led_torch_on = false;
        m_sos_on = false;
    }

    if (m_screen) {
        lv_obj_del(m_screen);
        m_screen = nullptr;
    }
}

} // namespace Apps
} // namespace CydOS
