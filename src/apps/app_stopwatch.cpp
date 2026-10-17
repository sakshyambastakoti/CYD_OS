/**
 * @file app_stopwatch.cpp
 * @brief Stopwatch & Timer Application implementation
 */

#include "app_stopwatch.h"
#include "ui/theme.h"
#include "hal/audio_hal.h"
#include <Arduino.h>
#include <stdio.h>

namespace CydOS {
namespace Apps {

StopwatchApp::StopwatchApp()
    : m_screen(nullptr), m_lbl_time(nullptr), m_btn_start_stop(nullptr),
      m_lbl_start_stop(nullptr), m_lbl_laps(nullptr),
      m_running(false), m_start_time(0), m_elapsed_time(0),
      m_last_lap_time(0), m_lap_count(0) {
    m_laps_text[0] = '\0';
}

StopwatchApp::~StopwatchApp() {
    destroy_screen();
}

void StopwatchApp::start_stop_cb(lv_event_t* e) {
    HAL::audio_play_click();
    StopwatchApp* app = (StopwatchApp*)lv_event_get_user_data(e);
    app->m_running = !app->m_running;

    if (app->m_running) {
        app->m_start_time = millis() - app->m_elapsed_time;
        lv_label_set_text(app->m_lbl_start_stop, "PAUSE");
        lv_obj_set_style_bg_color(app->m_btn_start_stop, THEME_COLOR_WARN, 0);
    } else {
        lv_label_set_text(app->m_lbl_start_stop, "START");
        lv_obj_set_style_bg_color(app->m_btn_start_stop, THEME_COLOR_PRIMARY, 0);
    }
}

void StopwatchApp::reset_lap_cb(lv_event_t* e) {
    HAL::audio_play_click();
    StopwatchApp* app = (StopwatchApp*)lv_event_get_user_data(e);

    if (app->m_running) {
        // Record Lap
        if (app->m_lap_count < 4) {
            app->m_lap_count++;
            uint32_t total = app->m_elapsed_time;
            uint32_t m = total / 60000;
            uint32_t s = (total % 60000) / 1000;
            uint32_t ms = (total % 1000) / 100;
            char lap_buf[32];
            snprintf(lap_buf, sizeof(lap_buf), "Lap %d: %02u:%02u.%u\n", app->m_lap_count, m, s, ms);
            strncat(app->m_laps_text, lap_buf, sizeof(app->m_laps_text) - strlen(app->m_laps_text) - 1);
            if (app->m_lbl_laps) lv_label_set_text(app->m_lbl_laps, app->m_laps_text);
        }
    } else {
        // Reset
        app->m_elapsed_time = 0;
        app->m_lap_count = 0;
        app->m_laps_text[0] = '\0';
        if (app->m_lbl_time) lv_label_set_text(app->m_lbl_time, "00:00.0");
        if (app->m_lbl_laps) lv_label_set_text(app->m_lbl_laps, "No laps recorded");
    }
}

void StopwatchApp::update() {
    if (m_running && m_lbl_time) {
        m_elapsed_time = millis() - m_start_time;
        uint32_t m = m_elapsed_time / 60000;
        uint32_t s = (m_elapsed_time % 60000) / 1000;
        uint32_t ms = (m_elapsed_time % 1000) / 100;

        char buf[16];
        snprintf(buf, sizeof(buf), "%02u:%02u.%u", m, s, ms);
        lv_label_set_text(m_lbl_time, buf);
    }
}

lv_obj_t* StopwatchApp::create_screen() {
    m_screen = lv_obj_create(nullptr);
    lv_obj_add_style(m_screen, &UI::style_screen_bg, 0);
    lv_obj_set_size(m_screen, 240, 320);
    lv_obj_set_pos(m_screen, 0, 0);
    lv_obj_clear_flag(m_screen, LV_OBJ_FLAG_SCROLLABLE);

    // Timer Card (below status bar)
    lv_obj_t* card = lv_obj_create(m_screen);
    lv_obj_add_style(card, &UI::style_card, 0);
    lv_obj_set_size(card, 224, 76);
    lv_obj_set_pos(card, 8, 26);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    m_lbl_time = lv_label_create(card);
    lv_obj_set_style_text_color(m_lbl_time, THEME_COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(m_lbl_time, &lv_font_montserrat_24, 0);
    lv_obj_center(m_lbl_time);
    lv_label_set_text(m_lbl_time, "00:00.0");

    // Action Buttons
    m_btn_start_stop = lv_btn_create(m_screen);
    lv_obj_add_style(m_btn_start_stop, &UI::style_btn, 0);
    lv_obj_set_size(m_btn_start_stop, 108, 42);
    lv_obj_set_pos(m_btn_start_stop, 8, 108);
    lv_obj_set_style_bg_color(m_btn_start_stop, THEME_COLOR_PRIMARY, 0);
    lv_obj_add_event_cb(m_btn_start_stop, start_stop_cb, LV_EVENT_CLICKED, this);
    m_lbl_start_stop = lv_label_create(m_btn_start_stop);
    lv_label_set_text(m_lbl_start_stop, "START");
    lv_obj_set_style_text_color(m_lbl_start_stop, THEME_COLOR_BG, 0);
    lv_obj_set_style_text_font(m_lbl_start_stop, &lv_font_montserrat_14, 0);
    lv_obj_center(m_lbl_start_stop);

    lv_obj_t* btn_reset = lv_btn_create(m_screen);
    lv_obj_add_style(btn_reset, &UI::style_btn, 0);
    lv_obj_set_size(btn_reset, 108, 42);
    lv_obj_set_pos(btn_reset, 124, 108);
    lv_obj_add_event_cb(btn_reset, reset_lap_cb, LV_EVENT_CLICKED, this);
    lv_obj_t* lbl_r = lv_label_create(btn_reset);
    lv_label_set_text(lbl_r, "LAP / RESET");
    lv_obj_set_style_text_color(lbl_r, THEME_COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(lbl_r, &lv_font_montserrat_12, 0);
    lv_obj_center(lbl_r);

    // Laps card
    lv_obj_t* laps_card = lv_obj_create(m_screen);
    lv_obj_add_style(laps_card, &UI::style_card, 0);
    lv_obj_set_size(laps_card, 224, 130);
    lv_obj_set_pos(laps_card, 8, 156);
    lv_obj_clear_flag(laps_card, LV_OBJ_FLAG_SCROLLABLE);

    m_lbl_laps = lv_label_create(laps_card);
    lv_obj_add_style(m_lbl_laps, &UI::style_subtitle, 0);
    lv_obj_set_pos(m_lbl_laps, 4, 4);
    lv_label_set_text(m_lbl_laps, m_lap_count > 0 ? m_laps_text : "Tap LAP to split times");

    return m_screen;
}

void StopwatchApp::destroy_screen() {
    if (m_screen) {
        lv_obj_del(m_screen);
        m_screen = nullptr;
        m_lbl_time = nullptr;
        m_btn_start_stop = nullptr;
        m_lbl_start_stop = nullptr;
        m_lbl_laps = nullptr;
    }
}

} // namespace Apps
} // namespace CydOS
