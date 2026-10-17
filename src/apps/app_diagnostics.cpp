/**
 * @file app_diagnostics.cpp
 * @brief Hardware Diagnostics Application implementation
 */

#include "app_diagnostics.h"
#include "ui/theme.h"
#include "hal/adc_hal.h"
#include "hal/sd_hal.h"
#include "hal/touch_hal.h"
#include <Arduino.h>

namespace CydOS {
namespace Apps {

DiagnosticsApp::DiagnosticsApp()
    : m_screen(nullptr), m_lbl_heap(nullptr), m_lbl_ldr(nullptr),
      m_lbl_sd(nullptr), m_lbl_touch(nullptr), m_lbl_uptime(nullptr) {
}

DiagnosticsApp::~DiagnosticsApp() {
    destroy_screen();
}

void DiagnosticsApp::update() {
    if (m_screen == nullptr) return;

    // 1. Heap
    if (m_lbl_heap) {
        lv_label_set_text_fmt(m_lbl_heap, "Heap: %u KB free / %u KB min", 
                              ESP.getFreeHeap() / 1024, ESP.getMinFreeHeap() / 1024);
    }

    // 2. LDR
    if (m_lbl_ldr) {
        lv_label_set_text_fmt(m_lbl_ldr, "LDR Ambient Light: %d%%", HAL::adc_read_ldr_percent());
    }

    // 3. Touch
    if (m_lbl_touch) {
        if (HAL::touch_is_pressed()) {
            lv_label_set_text_fmt(m_lbl_touch, "Touch: PRESSED (X:%d, Y:%d)", 
                                  HAL::touch_get_x(), HAL::touch_get_y());
            lv_obj_set_style_text_color(m_lbl_touch, UI::THEME_COLOR_PRIMARY, 0);
        } else {
            lv_label_set_text(m_lbl_touch, "Touch: Released");
            lv_obj_set_style_text_color(m_lbl_touch, UI::THEME_COLOR_TEXT_MUTED, 0);
        }
    }

    // 4. Uptime
    if (m_lbl_uptime) {
        uint32_t s = millis() / 1000;
        uint32_t m = s / 60;
        s %= 60;
        lv_label_set_text_fmt(m_lbl_uptime, "System Uptime: %02u:%02u", m, s);
    }
}

lv_obj_t* DiagnosticsApp::create_screen() {
    m_screen = lv_obj_create(nullptr);
    lv_obj_add_style(m_screen, &UI::style_screen_bg, 0);
    lv_obj_set_size(m_screen, 240, 274);
    lv_obj_set_pos(m_screen, 0, 22);
    lv_obj_clear_flag(m_screen, LV_OBJ_FLAG_SCROLLABLE);

    // Header Card
    lv_obj_t* card_header = lv_obj_create(m_screen);
    lv_obj_add_style(card_header, &UI::style_card, 0);
    lv_obj_set_size(card_header, 224, 46);
    lv_obj_set_pos(card_header, 8, 4);
    lv_obj_clear_flag(card_header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lbl_title = lv_label_create(card_header);
    lv_label_set_text(lbl_title, LV_SYMBOL_SETTINGS " Hardware Diagnostics");
    lv_obj_add_style(lbl_title, &UI::style_title, 0);
    lv_obj_align(lbl_title, LV_ALIGN_LEFT_MID, 0, 0);

    // Diagnostics Card
    lv_obj_t* card = lv_obj_create(m_screen);
    lv_obj_add_style(card, &UI::style_card, 0);
    lv_obj_set_size(card, 224, 210);
    lv_obj_set_pos(card, 8, 56);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    m_lbl_heap = lv_label_create(card);
    lv_obj_add_style(m_lbl_heap, &UI::style_subtitle, 0);
    lv_obj_set_pos(m_lbl_heap, 4, 8);

    m_lbl_ldr = lv_label_create(card);
    lv_obj_add_style(m_lbl_ldr, &UI::style_subtitle, 0);
    lv_obj_set_pos(m_lbl_ldr, 4, 40);

    m_lbl_sd = lv_label_create(card);
    lv_obj_add_style(m_lbl_sd, &UI::style_subtitle, 0);
    lv_obj_set_pos(m_lbl_sd, 4, 72);
    if (HAL::sd_is_mounted()) {
        lv_label_set_text_fmt(m_lbl_sd, "MicroSD: Mounted (%llu MB)", 
                              HAL::sd_get_total_bytes() / (1024 * 1024));
        lv_obj_set_style_text_color(m_lbl_sd, UI::THEME_COLOR_PRIMARY, 0);
    } else {
        lv_label_set_text(m_lbl_sd, "MicroSD: Not Detected");
        lv_obj_set_style_text_color(m_lbl_sd, UI::THEME_COLOR_WARN, 0);
    }

    m_lbl_touch = lv_label_create(card);
    lv_obj_add_style(m_lbl_touch, &UI::style_subtitle, 0);
    lv_obj_set_pos(m_lbl_touch, 4, 104);

    m_lbl_uptime = lv_label_create(card);
    lv_obj_add_style(m_lbl_uptime, &UI::style_subtitle, 0);
    lv_obj_set_pos(m_lbl_uptime, 4, 136);

    lv_obj_t* lbl_board = lv_label_create(card);
    lv_obj_add_style(lbl_board, &UI::style_subtitle, 0);
    lv_obj_set_pos(lbl_board, 4, 168);
    lv_label_set_text(lbl_board, "Board: CYD (ST7789 + XPT2046)");

    update();
    return m_screen;
}

void DiagnosticsApp::destroy_screen() {
    if (m_screen) {
        lv_obj_del(m_screen);
        m_screen = nullptr;
        m_lbl_heap = nullptr;
        m_lbl_ldr = nullptr;
        m_lbl_sd = nullptr;
        m_lbl_touch = nullptr;
        m_lbl_uptime = nullptr;
    }
}

} // namespace Apps
} // namespace CydOS
