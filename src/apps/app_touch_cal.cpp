/**
 * @file app_touch_cal.cpp
 * @brief Interactive Touch Calibration & Accuracy Verification Utility
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
      m_btn_start(nullptr), m_btn_test(nullptr), m_btn_reset(nullptr),
      m_calibrating(false), m_test_overlay(nullptr),
      m_lbl_test_coords(nullptr), m_cursor_dot(nullptr) {
    for (int i = 0; i < 5; i++) {
        m_targets[i] = nullptr;
        m_target_hit[i] = false;
    }
}

TouchCalApp::~TouchCalApp() {
    destroy_screen();
}

void TouchCalApp::start_calibration_cb(lv_event_t* e) {
    TouchCalApp* app = (TouchCalApp*)lv_event_get_user_data(e);
    app->run_calibration();
}

void TouchCalApp::open_test_cb(lv_event_t* e) {
    HAL::audio_play_click();
    TouchCalApp* app = (TouchCalApp*)lv_event_get_user_data(e);
    app->open_touch_test();
}

void TouchCalApp::close_test_cb(lv_event_t* e) {
    HAL::audio_play_click();
    TouchCalApp* app = (TouchCalApp*)lv_event_get_user_data(e);
    app->close_touch_test();
}

void TouchCalApp::reset_calibration_cb(lv_event_t* e) {
    TouchCalApp* app = (TouchCalApp*)lv_event_get_user_data(e);
    HAL::audio_play_click();

    auto& settings = Services::storage_get_settings();
    settings.touch_calibrated = false;
    memset(settings.touch_cal, 0, sizeof(settings.touch_cal));
    Services::storage_save();

    // Re-apply default native hardware calibration profile
    uint16_t default_cal[8] = { 300, 3700, 300, 200, 3900, 3700, 3900, 200 };
    HAL::touch_set_calibration(default_cal);

    if (app->m_lbl_status) {
        lv_label_set_text(app->m_lbl_status, "Status: Factory Defaults (Active)\nTouch bounds restored.");
        lv_obj_set_style_text_color(app->m_lbl_status, THEME_COLOR_WARN, 0);
    }
}

void TouchCalApp::target_click_cb(lv_event_t* e) {
    TouchCalApp* app = (TouchCalApp*)lv_event_get_user_data(e);
    lv_obj_t* btn = lv_event_get_target(e);
    int id = (int)(intptr_t)lv_obj_get_user_data(btn);

    if (id >= 0 && id < 5) {
        app->m_target_hit[id] = true;
        lv_obj_set_style_bg_color(btn, THEME_COLOR_SUCCESS, 0);
        lv_obj_t* lbl = lv_obj_get_child(btn, 0);
        if (lbl) {
            lv_label_set_text(lbl, LV_SYMBOL_OK);
        }

        // Check if all 5 targets are hit
        bool all_hit = true;
        for (int i = 0; i < 5; i++) {
            if (!app->m_target_hit[i]) {
                all_hit = false;
                break;
            }
        }

        if (all_hit) {
            HAL::audio_play_notification();
            if (app->m_lbl_test_coords) {
                lv_label_set_text(app->m_lbl_test_coords, "PERFECT! ALL 5 ZONES HIT");
                lv_obj_set_style_text_color(app->m_lbl_test_coords, THEME_COLOR_SUCCESS, 0);
            }
        } else {
            HAL::audio_play_click();
        }
    }
}

void TouchCalApp::run_calibration() {
    HAL::audio_play_click();
    m_calibrating = true;

    // Suspend LVGL touch sampling during calibration to prevent SPI bus contention
    HAL::touch_start_calibration();

    // Direct LovyanGFX interactive 4-corner calibration:
    // Crosshairs are drawn on display, 8 samples per corner averaged, affine matrix computed.
    uint16_t cal[8];
    g_display.calibrateTouch(cal, TFT_CYAN, TFT_BLACK, 14);

    // Resume LVGL touch input driver
    HAL::touch_finish_calibration();

    // Validate calibration output
    bool valid = true;
    for (int i = 0; i < 8; i++) {
        if (cal[i] < 100 || cal[i] > 4000) {
            valid = false;
            break;
        }
    }

    if (valid) {
        // Persist calibrated matrix to NVS flash
        auto& settings = Services::storage_get_settings();
        settings.touch_calibrated = true;
        memcpy(settings.touch_cal, cal, sizeof(cal));
        Services::storage_save();

        HAL::audio_play_notification();
        Serial.printf("[TOUCHCAL] Success! Calibration stored in NVS: [%u, %u, %u, %u, %u, %u, %u, %u]\n",
                      cal[0], cal[1], cal[2], cal[3], cal[4], cal[5], cal[6], cal[7]);

        if (m_lbl_status) {
            lv_label_set_text(m_lbl_status, "Status: Calibrated (Active)\nSaved to Flash!");
            lv_obj_set_style_text_color(m_lbl_status, THEME_COLOR_PRIMARY, 0);
        }
    } else {
        Serial.println("[TOUCHCAL] Calibration cancelled or invalid samples, keeping previous.");
    }

    m_calibrating = false;

    // Trigger full redraw of current screen
    lv_obj_invalidate(lv_scr_act());
}

void TouchCalApp::open_touch_test() {
    close_touch_test();

    m_test_overlay = lv_obj_create(m_screen);
    lv_obj_add_style(m_test_overlay, &UI::style_screen_bg, 0);
    lv_obj_set_size(m_test_overlay, 240, 320);
    lv_obj_set_pos(m_test_overlay, 0, 0);
    lv_obj_clear_flag(m_test_overlay, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Header Card
    lv_obj_t* card_header = lv_obj_create(m_test_overlay);
    lv_obj_add_style(card_header, &UI::style_card, 0);
    lv_obj_set_size(card_header, 224, 38);
    lv_obj_set_pos(card_header, 8, 24);
    lv_obj_clear_flag(card_header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lbl_t = lv_label_create(card_header);
    lv_label_set_text(lbl_t, LV_SYMBOL_IMAGE " Touch Accuracy Test");
    lv_obj_add_style(lbl_t, &UI::style_title, 0);
    lv_obj_align(lbl_t, LV_ALIGN_LEFT_MID, 0, 0);

    // 2. Readout Badge
    lv_obj_t* card_readout = lv_obj_create(m_test_overlay);
    lv_obj_add_style(card_readout, &UI::style_card, 0);
    lv_obj_set_size(card_readout, 224, 28);
    lv_obj_set_pos(card_readout, 8, 66);
    lv_obj_clear_flag(card_readout, LV_OBJ_FLAG_SCROLLABLE);

    m_lbl_test_coords = lv_label_create(card_readout);
    lv_label_set_text(m_lbl_test_coords, "Tap the 5 targets or drag");
    lv_obj_add_style(m_lbl_test_coords, &UI::style_subtitle, 0);
    lv_obj_align(m_lbl_test_coords, LV_ALIGN_CENTER, 0, 0);

    // 3. Targets: 0:TL, 1:TR, 2:Center, 3:BL, 4:BR
    const char* names[5] = { "TL", "TR", "CENTER", "BL", "BR" };
    int16_t coords[5][4] = {
        { 8,   98, 64, 46 },  // TL
        { 168, 98, 64, 46 },  // TR
        { 74, 152, 92, 46 },  // CENTER
        { 8,  206, 64, 46 },  // BL
        { 168, 206, 64, 46 }  // BR
    };

    for (int i = 0; i < 5; i++) {
        m_target_hit[i] = false;
        m_targets[i] = lv_btn_create(m_test_overlay);
        lv_obj_add_style(m_targets[i], &UI::style_btn, 0);
        lv_obj_set_size(m_targets[i], coords[i][2], coords[i][3]);
        lv_obj_set_pos(m_targets[i], coords[i][0], coords[i][1]);
        lv_obj_set_style_bg_color(m_targets[i], THEME_COLOR_SURFACE, 0);
        lv_obj_set_style_border_color(m_targets[i], THEME_COLOR_PRIMARY, 0);
        lv_obj_set_style_border_width(m_targets[i], 2, 0);
        lv_obj_set_user_data(m_targets[i], (void*)(intptr_t)i);
        lv_obj_add_event_cb(m_targets[i], target_click_cb, LV_EVENT_CLICKED, this);

        lv_obj_t* lbl = lv_label_create(m_targets[i]);
        lv_label_set_text(lbl, names[i]);
        lv_obj_set_style_text_color(lbl, THEME_COLOR_PRIMARY, 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
        lv_obj_center(lbl);
    }

    // 4. Floating Cursor Dot (tracks finger live)
    m_cursor_dot = lv_obj_create(m_test_overlay);
    lv_obj_set_size(m_cursor_dot, 16, 16);
    lv_obj_set_style_radius(m_cursor_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(m_cursor_dot, THEME_COLOR_PRIMARY, 0);
    lv_obj_set_style_border_color(m_cursor_dot, lv_color_white(), 0);
    lv_obj_set_style_border_width(m_cursor_dot, 2, 0);
    lv_obj_clear_flag(m_cursor_dot, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(m_cursor_dot, LV_OBJ_FLAG_HIDDEN);

    // 5. Exit Button
    lv_obj_t* btn_exit = lv_btn_create(m_test_overlay);
    lv_obj_add_style(btn_exit, &UI::style_btn, 0);
    lv_obj_set_size(btn_exit, 224, 40);
    lv_obj_set_pos(btn_exit, 8, 260);
    lv_obj_set_style_bg_color(btn_exit, THEME_COLOR_PRIMARY, 0);
    lv_obj_add_event_cb(btn_exit, close_test_cb, LV_EVENT_CLICKED, this);

    lv_obj_t* lbl_exit = lv_label_create(btn_exit);
    lv_label_set_text(lbl_exit, LV_SYMBOL_LEFT "  Done / Return to Menu");
    lv_obj_set_style_text_color(lbl_exit, THEME_COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(lbl_exit, &lv_font_montserrat_14, 0);
    lv_obj_center(lbl_exit);
}

void TouchCalApp::close_touch_test() {
    if (m_test_overlay) {
        lv_obj_del(m_test_overlay);
        m_test_overlay = nullptr;
        m_lbl_test_coords = nullptr;
        m_cursor_dot = nullptr;
        for (int i = 0; i < 5; i++) {
            m_targets[i] = nullptr;
        }
    }
}

void TouchCalApp::update() {
    if (m_test_overlay && m_lbl_test_coords) {
        if (HAL::touch_is_pressed()) {
            int16_t x = HAL::touch_get_x();
            int16_t y = HAL::touch_get_y();
            lv_label_set_text_fmt(m_lbl_test_coords, "Touch: (%d, %d)", x, y);

            if (m_cursor_dot) {
                lv_obj_clear_flag(m_cursor_dot, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_pos(m_cursor_dot, x - 8, y - 8);
            }
        } else {
            if (m_cursor_dot) {
                lv_obj_add_flag(m_cursor_dot, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
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
    lv_obj_set_size(card_info, 224, 86);
    lv_obj_set_pos(card_info, 8, 76);
    lv_obj_clear_flag(card_info, LV_OBJ_FLAG_SCROLLABLE);

    m_lbl_status = lv_label_create(card_info);
    lv_obj_add_style(m_lbl_status, &UI::style_subtitle, 0);
    lv_obj_set_pos(m_lbl_status, 4, 4);

    auto& settings = Services::storage_get_settings();
    if (settings.touch_calibrated) {
        lv_label_set_text(m_lbl_status, "Status: Calibrated (Active)\nCustom bounds loaded.");
        lv_obj_set_style_text_color(m_lbl_status, THEME_COLOR_PRIMARY, 0);
    } else {
        lv_label_set_text(m_lbl_status, "Status: Factory Defaults\nVerified profile active.");
        lv_obj_set_style_text_color(m_lbl_status, THEME_COLOR_WARN, 0);
    }

    lv_obj_t* lbl_instructions = lv_label_create(card_info);
    lv_obj_set_style_text_color(lbl_instructions, THEME_COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(lbl_instructions, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(lbl_instructions, 4, 44);
    lv_label_set_text(lbl_instructions, "Verify accuracy with Live Test\nor recalibrate with 4-Corner.");

    // 3. Live Accuracy Test Button
    m_btn_test = lv_btn_create(m_screen);
    lv_obj_add_style(m_btn_test, &UI::style_btn, 0);
    lv_obj_set_size(m_btn_test, 224, 40);
    lv_obj_set_pos(m_btn_test, 8, 168);
    lv_obj_set_style_bg_color(m_btn_test, THEME_COLOR_PRIMARY, 0);
    lv_obj_add_event_cb(m_btn_test, open_test_cb, LV_EVENT_CLICKED, this);

    lv_obj_t* lbl_btn_test = lv_label_create(m_btn_test);
    lv_label_set_text(lbl_btn_test, LV_SYMBOL_IMAGE "  Live Accuracy & Target Test");
    lv_obj_set_style_text_color(lbl_btn_test, THEME_COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(lbl_btn_test, &lv_font_montserrat_14, 0);
    lv_obj_center(lbl_btn_test);

    // 4. Start 4-Corner Calibration Button
    m_btn_start = lv_btn_create(m_screen);
    lv_obj_add_style(m_btn_start, &UI::style_btn, 0);
    lv_obj_set_size(m_btn_start, 224, 40);
    lv_obj_set_pos(m_btn_start, 8, 214);
    lv_obj_set_style_bg_color(m_btn_start, THEME_COLOR_SURFACE, 0);
    lv_obj_set_style_border_color(m_btn_start, THEME_COLOR_PRIMARY, 0);
    lv_obj_set_style_border_width(m_btn_start, 1, 0);
    lv_obj_add_event_cb(m_btn_start, start_calibration_cb, LV_EVENT_CLICKED, this);

    lv_obj_t* lbl_btn_start = lv_label_create(m_btn_start);
    lv_label_set_text(lbl_btn_start, LV_SYMBOL_PLAY "  Start 4-Corner Calibration");
    lv_obj_set_style_text_color(lbl_btn_start, THEME_COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_btn_start, &lv_font_montserrat_14, 0);
    lv_obj_center(lbl_btn_start);

    // 5. Reset Button
    m_btn_reset = lv_btn_create(m_screen);
    lv_obj_add_style(m_btn_reset, &UI::style_btn, 0);
    lv_obj_set_size(m_btn_reset, 224, 38);
    lv_obj_set_pos(m_btn_reset, 8, 260);
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
    close_touch_test();
    if (m_screen) {
        lv_obj_del(m_screen);
        m_screen = nullptr;
        m_lbl_status = nullptr;
        m_btn_start = nullptr;
        m_btn_test = nullptr;
        m_btn_reset = nullptr;
    }
}

} // namespace Apps
} // namespace CydOS
