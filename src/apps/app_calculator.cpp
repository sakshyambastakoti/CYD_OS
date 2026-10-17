/**
 * @file app_calculator.cpp
 * @brief Smartphone Calculator Application implementation
 */

#include "app_calculator.h"
#include "ui/theme.h"
#include "hal/audio_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace CydOS {
namespace Apps {

CalculatorApp::CalculatorApp() 
    : m_screen(nullptr), m_lbl_display(nullptr), 
      m_accum(0.0), m_operand(0.0), m_pending_op(0), m_new_number(true) {
    strcpy(m_display_buf, "0");
}

CalculatorApp::~CalculatorApp() {
    destroy_screen();
}

void CalculatorApp::btn_click_cb(lv_event_t* e) {
    HAL::audio_play_click();
    CalculatorApp* app = (CalculatorApp*)lv_event_get_user_data(e);
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* lbl = lv_obj_get_child(btn, 0);
    const char* txt = lv_label_get_text(lbl);
    app->handle_input(txt);
}

void CalculatorApp::handle_input(const char* key) {
    if (strcmp(key, "C") == 0) {
        m_accum = 0.0;
        m_operand = 0.0;
        m_pending_op = 0;
        m_new_number = true;
        strcpy(m_display_buf, "0");
    } else if (strcmp(key, "+/-") == 0) {
        double val = atof(m_display_buf);
        val = -val;
        snprintf(m_display_buf, sizeof(m_display_buf), "%.4g", val);
    } else if (strcmp(key, "+") == 0 || strcmp(key, "-") == 0 || 
               strcmp(key, "*") == 0 || strcmp(key, "/") == 0) {
        m_accum = atof(m_display_buf);
        m_pending_op = key[0];
        m_new_number = true;
    } else if (strcmp(key, "=") == 0) {
        if (m_pending_op != 0) {
            double cur = atof(m_display_buf);
            double res = m_accum;
            if (m_pending_op == '+') res = m_accum + cur;
            else if (m_pending_op == '-') res = m_accum - cur;
            else if (m_pending_op == '*') res = m_accum * cur;
            else if (m_pending_op == '/') {
                if (cur != 0.0) res = m_accum / cur;
                else res = 0.0; // Avoid divide by zero crash
            }
            snprintf(m_display_buf, sizeof(m_display_buf), "%.5g", res);
            m_accum = res;
            m_pending_op = 0;
            m_new_number = true;
        }
    } else if (strcmp(key, ".") == 0) {
        if (m_new_number) {
            strcpy(m_display_buf, "0.");
            m_new_number = false;
        } else if (strchr(m_display_buf, '.') == nullptr) {
            strncat(m_display_buf, ".", sizeof(m_display_buf) - strlen(m_display_buf) - 1);
        }
    } else {
        // Digit 0-9
        if (m_new_number || strcmp(m_display_buf, "0") == 0) {
            strncpy(m_display_buf, key, sizeof(m_display_buf) - 1);
            m_new_number = false;
        } else {
            if (strlen(m_display_buf) < 10) {
                strncat(m_display_buf, key, sizeof(m_display_buf) - strlen(m_display_buf) - 1);
            }
        }
    }
    update_display();
}

void CalculatorApp::update_display() {
    if (m_lbl_display) {
        lv_label_set_text(m_lbl_display, m_display_buf);
    }
}

lv_obj_t* CalculatorApp::create_screen() {
    m_screen = lv_obj_create(nullptr);
    lv_obj_add_style(m_screen, &UI::style_screen_bg, 0);
    lv_obj_set_size(m_screen, 240, 274);
    lv_obj_set_pos(m_screen, 0, 22);
    lv_obj_clear_flag(m_screen, LV_OBJ_FLAG_SCROLLABLE);

    // Display readout card
    lv_obj_t* display_card = lv_obj_create(m_screen);
    lv_obj_add_style(display_card, &UI::style_card, 0);
    lv_obj_set_size(display_card, 224, 46);
    lv_obj_set_pos(display_card, 8, 4);
    lv_obj_clear_flag(display_card, LV_OBJ_FLAG_SCROLLABLE);

    m_lbl_display = lv_label_create(display_card);
    lv_obj_set_style_text_color(m_lbl_display, UI::THEME_COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(m_lbl_display, &lv_font_montserrat_20, 0);
    lv_obj_align(m_lbl_display, LV_ALIGN_RIGHT_MID, -4, 0);
    lv_label_set_text(m_lbl_display, m_display_buf);

    // Keypad layout
    const char* keys[5][4] = {
        {"C", "+/-", "%", "/"},
        {"7", "8", "9", "*"},
        {"4", "5", "6", "-"},
        {"1", "2", "3", "+"},
        {"0", ".", "=", "="}
    };

    uint16_t start_y = 56;
    uint16_t start_x = 8;
    uint16_t btn_w = 52;
    uint16_t btn_h = 38;
    uint16_t spacing = 5;

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 4; c++) {
            if (r == 4 && c == 3) continue; // Skip redundant 2nd '='

            lv_obj_t* btn = lv_btn_create(m_screen);
            lv_obj_add_style(btn, &UI::style_btn, 0);

            if (r == 4 && c == 2) {
                // Wide '=' button
                lv_obj_set_size(btn, btn_w * 2 + spacing, btn_h);
                lv_obj_set_pos(btn, start_x + c * (btn_w + spacing), start_y + r * (btn_h + spacing));
                lv_obj_set_style_bg_color(btn, UI::THEME_COLOR_PRIMARY, 0);
            } else {
                lv_obj_set_size(btn, btn_w, btn_h);
                lv_obj_set_pos(btn, start_x + c * (btn_w + spacing), start_y + r * (btn_h + spacing));

                if (c == 3) {
                    lv_obj_set_style_bg_color(btn, UI::THEME_COLOR_SECONDARY, 0);
                } else if (r == 0 && c == 0) {
                    lv_obj_set_style_bg_color(btn, UI::THEME_COLOR_DANGER, 0);
                }
            }

            lv_obj_add_event_cb(btn, btn_click_cb, LV_EVENT_CLICKED, this);

            lv_obj_t* lbl = lv_label_create(btn);
            lv_label_set_text(lbl, keys[r][c]);
            lv_obj_set_style_text_color(lbl, UI::THEME_COLOR_TEXT_MAIN, 0);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
            lv_obj_center(lbl);
        }
    }

    return m_screen;
}

void CalculatorApp::destroy_screen() {
    if (m_screen) {
        lv_obj_del(m_screen);
        m_screen = nullptr;
        m_lbl_display = nullptr;
    }
}

} // namespace Apps
} // namespace CydOS
