/**
 * @file app_settings.cpp
 * @brief System Settings Application implementation
 */

#include "app_settings.h"
#include "ui/theme.h"
#include "ui/app_manager.h"
#include "hal/display_hal.h"
#include "hal/audio_hal.h"
#include "hal/power_hal.h"
#include "services/storage_service.h"
#include "app_config.h"
#include <Arduino.h>

namespace CydOS {
namespace Apps {

SettingsApp::SettingsApp() : m_screen(nullptr), m_slider_bright(nullptr),
                             m_slider_vol(nullptr), m_dd_timeout(nullptr) {
}

SettingsApp::~SettingsApp() {
    destroy_screen();
}

void SettingsApp::bright_slider_cb(lv_event_t* e) {
    SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
    int32_t val = lv_slider_get_value(app->m_slider_bright);
    HAL::display_set_brightness((uint8_t)val);
    Services::storage_get_settings().brightness = (uint8_t)val;
}

void SettingsApp::vol_slider_cb(lv_event_t* e) {
    SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
    int32_t val = lv_slider_get_value(app->m_slider_vol);
    HAL::audio_set_volume((uint8_t)val);
    Services::storage_get_settings().volume = (uint8_t)val;
    HAL::audio_play_click();
}

void SettingsApp::timeout_dd_cb(lv_event_t* e) {
    SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
    uint16_t opt = lv_dropdown_get_selected(app->m_dd_timeout);
    uint32_t sec = 30;
    if (opt == 0) sec = 15;
    else if (opt == 1) sec = 30;
    else if (opt == 2) sec = 60;
    else if (opt == 3) sec = 0; // Never

    HAL::power_set_timeout(sec);
    Services::storage_get_settings().screen_timeout_sec = sec;
    HAL::audio_play_click();
}

void SettingsApp::touch_cal_btn_cb(lv_event_t* e) {
    HAL::audio_play_click();
    UI::app_manager_open(UI::APP_TOUCH_CAL);
}

void SettingsApp::save_btn_cb(lv_event_t* e) {
    HAL::audio_play_notification();
    Services::storage_save();
}

lv_obj_t* SettingsApp::create_screen() {
    m_screen = lv_obj_create(nullptr);
    lv_obj_add_style(m_screen, &UI::style_screen_bg, 0);
    lv_obj_set_size(m_screen, 240, 274);
    lv_obj_set_pos(m_screen, 0, 22);
    // Allow vertical scrolling
    lv_obj_set_scroll_dir(m_screen, LV_DIR_VER);

    // 1. Display Card
    lv_obj_t* card_disp = lv_obj_create(m_screen);
    lv_obj_add_style(card_disp, &UI::style_card, 0);
    lv_obj_set_size(card_disp, 224, 110);
    lv_obj_set_pos(card_disp, 8, 4);
    lv_obj_clear_flag(card_disp, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lbl_d = lv_label_create(card_disp);
    lv_label_set_text(lbl_d, LV_SYMBOL_EYE_OPEN " Display");
    lv_obj_add_style(lbl_d, &UI::style_title, 0);
    lv_obj_set_pos(lbl_d, 2, 0);

    lv_obj_t* lbl_b = lv_label_create(card_disp);
    lv_label_set_text(lbl_b, "Brightness");
    lv_obj_add_style(lbl_b, &UI::style_subtitle, 0);
    lv_obj_set_pos(lbl_b, 2, 22);

    m_slider_bright = lv_slider_create(card_disp);
    lv_slider_set_range(m_slider_bright, 15, 255);
    lv_slider_set_value(m_slider_bright, Services::storage_get_settings().brightness, LV_ANIM_OFF);
    lv_obj_set_size(m_slider_bright, 204, 14);
    lv_obj_set_pos(m_slider_bright, 2, 40);
    lv_obj_set_style_bg_color(m_slider_bright, UI::THEME_COLOR_PRIMARY, LV_PART_INDICATOR);
    lv_obj_add_event_cb(m_slider_bright, bright_slider_cb, LV_EVENT_VALUE_CHANGED, this);

    lv_obj_t* lbl_to = lv_label_create(card_disp);
    lv_label_set_text(lbl_to, "Sleep:");
    lv_obj_add_style(lbl_to, &UI::style_subtitle, 0);
    lv_obj_set_pos(lbl_to, 2, 68);

    m_dd_timeout = lv_dropdown_create(card_disp);
    lv_dropdown_set_options(m_dd_timeout, "15s\n30s\n60s\nNever");
    uint32_t cur_to = Services::storage_get_settings().screen_timeout_sec;
    if (cur_to == 15) lv_dropdown_set_selected(m_dd_timeout, 0);
    else if (cur_to == 30) lv_dropdown_set_selected(m_dd_timeout, 1);
    else if (cur_to == 60) lv_dropdown_set_selected(m_dd_timeout, 2);
    else if (cur_to == 0) lv_dropdown_set_selected(m_dd_timeout, 3);
    lv_obj_set_size(m_dd_timeout, 120, 32);
    lv_obj_set_pos(m_dd_timeout, 86, 62);
    lv_obj_add_event_cb(m_dd_timeout, timeout_dd_cb, LV_EVENT_VALUE_CHANGED, this);

    // 2. Sound Card
    lv_obj_t* card_snd = lv_obj_create(m_screen);
    lv_obj_add_style(card_snd, &UI::style_card, 0);
    lv_obj_set_size(card_snd, 224, 76);
    lv_obj_set_pos(card_snd, 8, 120);
    lv_obj_clear_flag(card_snd, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lbl_s = lv_label_create(card_snd);
    lv_label_set_text(lbl_s, LV_SYMBOL_VOLUME_MAX " Sound & Volume");
    lv_obj_add_style(lbl_s, &UI::style_title, 0);
    lv_obj_set_pos(lbl_s, 2, 0);

    m_slider_vol = lv_slider_create(card_snd);
    lv_slider_set_range(m_slider_vol, 0, 100);
    lv_slider_set_value(m_slider_vol, Services::storage_get_settings().volume, LV_ANIM_OFF);
    lv_obj_set_size(m_slider_vol, 204, 14);
    lv_obj_set_pos(m_slider_vol, 2, 28);
    lv_obj_set_style_bg_color(m_slider_vol, UI::THEME_COLOR_SECONDARY, LV_PART_INDICATOR);
    lv_obj_add_event_cb(m_slider_vol, vol_slider_cb, LV_EVENT_VALUE_CHANGED, this);

    // 3. Touch Calibration Card
    lv_obj_t* card_touch = lv_obj_create(m_screen);
    lv_obj_add_style(card_touch, &UI::style_card, 0);
    lv_obj_set_size(card_touch, 224, 76);
    lv_obj_set_pos(card_touch, 8, 202);
    lv_obj_clear_flag(card_touch, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lbl_tc = lv_label_create(card_touch);
    lv_label_set_text(lbl_tc, "Touchscreen Alignment");
    lv_obj_add_style(lbl_tc, &UI::style_title, 0);
    lv_obj_set_pos(lbl_tc, 2, 0);

    lv_obj_t* btn_cal = lv_btn_create(card_touch);
    lv_obj_add_style(btn_cal, &UI::style_btn, 0);
    lv_obj_set_size(btn_cal, 204, 30);
    lv_obj_set_pos(btn_cal, 2, 24);
    lv_obj_add_event_cb(btn_cal, touch_cal_btn_cb, LV_EVENT_CLICKED, this);
    lv_obj_t* lbl_btn_c = lv_label_create(btn_cal);
    lv_label_set_text(lbl_btn_c, "Calibrate Touch Screen");
    lv_obj_center(lbl_btn_c);

    // 4. Save Button
    lv_obj_t* btn_save = lv_btn_create(m_screen);
    lv_obj_add_style(btn_save, &UI::style_btn, 0);
    lv_obj_set_size(btn_save, 224, 36);
    lv_obj_set_pos(btn_save, 8, 284);
    lv_obj_set_style_bg_color(btn_save, UI::THEME_COLOR_PRIMARY, 0);
    lv_obj_add_event_cb(btn_save, save_btn_cb, LV_EVENT_CLICKED, this);
    lv_obj_t* lbl_save = lv_label_create(btn_save);
    lv_label_set_text(lbl_save, LV_SYMBOL_SAVE " Save Settings to Flash");
    lv_obj_set_style_text_color(lbl_save, UI::THEME_COLOR_BG, 0);
    lv_obj_center(lbl_save);

    // 5. About Device Card
    lv_obj_t* card_about = lv_obj_create(m_screen);
    lv_obj_add_style(card_about, &UI::style_card, 0);
    lv_obj_set_size(card_about, 224, 90);
    lv_obj_set_pos(card_about, 8, 326);
    lv_obj_clear_flag(card_about, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lbl_ab = lv_label_create(card_about);
    lv_label_set_text(lbl_ab, "About CYD OS");
    lv_obj_add_style(lbl_ab, &UI::style_title, 0);
    lv_obj_set_pos(lbl_ab, 2, 0);

    lv_obj_t* lbl_info = lv_label_create(card_about);
    lv_obj_add_style(lbl_info, &UI::style_subtitle, 0);
    lv_obj_set_pos(lbl_info, 2, 22);
    lv_label_set_text_fmt(lbl_info, 
        "Version: " CYD_OS_VERSION_STRING "\n"
        "Board: ESP32-2432S028R\n"
        "Free RAM: %u KB",
        ESP.getFreeHeap() / 1024);

    return m_screen;
}

void SettingsApp::destroy_screen() {
    if (m_screen) {
        lv_obj_del(m_screen);
        m_screen = nullptr;
    }
}

} // namespace Apps
} // namespace CydOS
