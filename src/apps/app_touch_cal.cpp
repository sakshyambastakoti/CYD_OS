/**
 * @file app_touch_cal.cpp
 * @brief Interactive 4-Corner Touch Calibration Utility using LovyanGFX
 */

#include "app_touch_cal.h"
#include "ui/theme.h"
#include "ui/app_manager.h"
#include "hal/touch_hal.h"
#include "hal/audio_hal.h"
#include "services/storage_service.h"
#include "cyd_config.h"
#include <Arduino.h>

extern LGFX_CYD g_display;

namespace CydOS {
namespace Apps {

TouchCalApp::TouchCalApp()
    : m_screen(nullptr), m_lbl_status(nullptr),
      m_btn_start(nullptr), m_btn_reset(nullptr), m_calibrating(false) {
}

TouchCalApp::~TouchCalApp() {
    destroy_screen();
}

void TouchCalApp::start_calibration_cb(lv_event_t* e) {
    TouchCalApp* app = (TouchCalApp*)lv_event_get_user_data(e);
    app->run_calibration();
}

void TouchCalApp::reset_calibration_cb(lv_event_t* e) {
    TouchCalApp* app = (TouchCalApp*)lv_event_get_user_data(e);
    HAL::audio_play_click();

    auto& settings = Services::storage_get_settings();
    settings.touch_calibrated = false;
    memset(settings.touch_cal, 0, sizeof(settings.touch_cal));
    Services::storage_save();

    // Re-apply default hardware calibration profile
    uint16_t default_cal[8] = { 300, 3700, 300, 200, 3900, 3700, 3900, 200 };
    HAL::touch_set_calibration(default_cal);

    if (app->m_lbl_status) {
        lv_label_set_text(app->m_lbl_status, "Status: Default Profile\nTap 'Start' to calibrate.");
        lv_obj_set_style_text_color(app->m_lbl_status, THEME_COLOR_WARN, 0);
    }
}

void TouchCalApp::run_calibration() {
    HAL::audio_play_click();
    m_calibrating = true;

    // Direct LovyanGFX interactive 4-corner calibration:
    // It temporarily cancels rotation offsets, displays crosshairs in each corner,
    // averages 8 samples per corner, computes the exact affine matrix, and applies it.
    uint16_t cal[8];
    g_display.calibrateTouch(cal, TFT_CYAN, TFT_BLACK, 14);

    // Persist calibrated matrix to NVS flash
    auto& settings = Services::storage_get_settings();
    settings.touch_calibrated = true;
    memcpy(settings.touch_cal, cal, sizeof(cal));
    Services::storage_save();

    HAL::audio_play_notification();
    Serial.printf("[TOUCHCAL] Success! Calibration stored in NVS: [%u, %u, %u, %u, %u, %u, %u, %u]\n",
                  cal[0], cal[1], cal[2], cal[3], cal[4], cal[5], cal[6], cal[7]);

    m_calibrating = false;

    // Invalidate active screen to trigger full LVGL re-render
    lv_obj_invalidate(lv_scr_act());

    if (m_lbl_status) {
        lv_label_set_text(m_lbl_status, "Status: Calibrated (Active)\nExact bounds saved to Flash!");
        lv_obj_set_style_text_color(m_lbl_status, THEME_COLOR_PRIMARY, 0);
    }
}

void TouchCalApp::update() {
    // No background polling needed while screen is active
}

lv_obj_t* TouchCalApp::create_screen() {
    m_screen = lv_obj_create(nullptr);
    lv_obj_add_style(m_screen, &UI::style_screen_bg, 0);
    lv_obj_set_size(m_screen, 240, 320);
    lv_obj_set_pos(m_screen, 0, 0);
    lv_obj_clear_flag(m_screen, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Header Card (below status bar)
    lv_obj_t* card_header = lv_obj_create(m_screen);
    lv_obj_add_style(card_header, &UI::style_card, 0);
    lv_obj_set_size(card_header, 224, 46);
    lv_obj_set_pos(card_header, 8, 26);
    lv_obj_clear_flag(card_header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lbl_title = lv_label_create(card_header);
    lv_label_set_text(lbl_title, LV_SYMBOL_PLUS " Touch Calibration");
    lv_obj_add_style(lbl_title, &UI::style_title, 0);
    lv_obj_align(lbl_title, LV_ALIGN_LEFT_MID, 0, 0);

    // 2. Info / Status Card
    lv_obj_t* card_info = lv_obj_create(m_screen);
    lv_obj_add_style(card_info, &UI::style_card, 0);
    lv_obj_set_size(card_info, 224, 120);
    lv_obj_set_pos(card_info, 8, 78);
    lv_obj_clear_flag(card_info, LV_OBJ_FLAG_SCROLLABLE);

    m_lbl_status = lv_label_create(card_info);
    lv_obj_add_style(m_lbl_status, &UI::style_subtitle, 0);
    lv_obj_set_pos(m_lbl_status, 4, 4);

    auto& settings = Services::storage_get_settings();
    if (settings.touch_calibrated) {
        lv_label_set_text(m_lbl_status, "Status: Calibrated (Active)\nCustom bounds loaded.");
        lv_obj_set_style_text_color(m_lbl_status, THEME_COLOR_PRIMARY, 0);
    } else {
        lv_label_set_text(m_lbl_status, "Status: Hardware Defaults\nTap 'Start' to calibrate.");
        lv_obj_set_style_text_color(m_lbl_status, THEME_COLOR_WARN, 0);
    }

    lv_obj_t* lbl_instructions = lv_label_create(card_info);
    lv_obj_set_style_text_color(lbl_instructions, THEME_COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(lbl_instructions, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lbl_instructions, 4, 48);
    lv_label_set_text(lbl_instructions, "Tap 'Start', then precisely\ntouch the 4 corner crosshairs\nwhen prompted.");

    // 3. Start Calibration Button
    m_btn_start = lv_btn_create(m_screen);
    lv_obj_add_style(m_btn_start, &UI::style_btn, 0);
    lv_obj_set_size(m_btn_start, 224, 44);
    lv_obj_set_pos(m_btn_start, 8, 206);
    lv_obj_set_style_bg_color(m_btn_start, THEME_COLOR_PRIMARY, 0);
    lv_obj_add_event_cb(m_btn_start, start_calibration_cb, LV_EVENT_CLICKED, this);

    lv_obj_t* lbl_btn_start = lv_label_create(m_btn_start);
    lv_label_set_text(lbl_btn_start, LV_SYMBOL_PLAY "  Start 4-Corner Calibration");
    lv_obj_set_style_text_color(lbl_btn_start, THEME_COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(lbl_btn_start, &lv_font_montserrat_14, 0);
    lv_obj_center(lbl_btn_start);

    // 4. Reset Button
    m_btn_reset = lv_btn_create(m_screen);
    lv_obj_add_style(m_btn_reset, &UI::style_btn, 0);
    lv_obj_set_size(m_btn_reset, 224, 40);
    lv_obj_set_pos(m_btn_reset, 8, 258);
    lv_obj_set_style_bg_color(m_btn_reset, THEME_COLOR_SURFACE, 0);
    lv_obj_set_style_border_color(m_btn_reset, THEME_COLOR_BORDER, 0);
    lv_obj_set_style_border_width(m_btn_reset, 1, 0);
    lv_obj_add_event_cb(m_btn_reset, reset_calibration_cb, LV_EVENT_CLICKED, this);

    lv_obj_t* lbl_btn_reset = lv_label_create(m_btn_reset);
    lv_label_set_text(lbl_btn_reset, LV_SYMBOL_REFRESH "  Reset to Factory Defaults");
    lv_obj_set_style_text_color(lbl_btn_reset, THEME_COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(lbl_btn_reset, &lv_font_montserrat_14, 0);
    lv_obj_center(lbl_btn_reset);

    return m_screen;
}

void TouchCalApp::destroy_screen() {
    if (m_screen) {
        lv_obj_del(m_screen);
        m_screen = nullptr;
        m_lbl_status = nullptr;
        m_btn_start = nullptr;
        m_btn_reset = nullptr;
    }
}

} // namespace Apps
} // namespace CydOS
