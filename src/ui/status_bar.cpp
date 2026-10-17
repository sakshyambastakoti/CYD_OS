/**
 * @file status_bar.cpp
 * @brief Persistent Top Status Bar implementation
 */

#include "status_bar.h"
#include "quick_settings.h"
#include "theme.h"
#include "services/time_service.h"
#include "services/storage_service.h"
#include "hal/sd_hal.h"
#include "hal/audio_hal.h"
#include <Arduino.h>

namespace CydOS {
namespace UI {

static lv_obj_t* s_bar_cont  = nullptr;
static lv_obj_t* s_lbl_time  = nullptr;
static lv_obj_t* s_lbl_icons = nullptr;

static void status_bar_click_event(lv_event_t* e) {
    HAL::audio_play_click();
    quick_settings_toggle();
}

void status_bar_init() {
    lv_obj_t* top = lv_layer_top();

    // Container for status bar: 240x22
    s_bar_cont = lv_obj_create(top);
    lv_obj_remove_style_all(s_bar_cont);
    lv_obj_set_size(s_bar_cont, 240, 22);
    lv_obj_set_pos(s_bar_cont, 0, 0);
    lv_obj_set_style_bg_color(s_bar_cont, lv_color_hex(0x0C0D14), 0);
    lv_obj_set_style_bg_opa(s_bar_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_bar_cont, 1, 0);
    lv_obj_set_style_border_color(s_bar_cont, THEME_COLOR_BORDER, 0);
    lv_obj_set_style_border_side(s_bar_cont, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_clear_flag(s_bar_cont, LV_OBJ_FLAG_SCROLLABLE);

    // Make status bar clickable to open quick settings shade
    lv_obj_add_flag(s_bar_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(s_bar_cont, status_bar_click_event, LV_EVENT_CLICKED, nullptr);

    // Time label on left
    s_lbl_time = lv_label_create(s_bar_cont);
    lv_obj_set_pos(s_lbl_time, 8, 3);
    lv_obj_set_style_text_color(s_lbl_time, THEME_COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(s_lbl_time, &lv_font_montserrat_12, 0);
    lv_label_set_text(s_lbl_time, "12:00");

    // Icons label on right
    s_lbl_icons = lv_label_create(s_bar_cont);
    lv_obj_align(s_lbl_icons, LV_ALIGN_TOP_RIGHT, -6, 3);
    lv_obj_set_style_text_color(s_lbl_icons, THEME_COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(s_lbl_icons, &lv_font_montserrat_12, 0);
    lv_label_set_text(s_lbl_icons, LV_SYMBOL_SD_CARD " " LV_SYMBOL_BATTERY_FULL);

    status_bar_update();
    Serial.println("[UI] Status bar mounted on top layer");
}

void status_bar_update() {
    if (s_lbl_time == nullptr || s_lbl_icons == nullptr) return;

    // 1. Time string
    char timeBuf[16];
    Services::time_get_formatted(timeBuf, sizeof(timeBuf), false);
    lv_label_set_text(s_lbl_time, timeBuf);

    // 2. Status icons
    char iconBuf[32];
    bool sdMounted = HAL::sd_is_mounted();
    snprintf(iconBuf, sizeof(iconBuf), "%s " LV_SYMBOL_BATTERY_FULL " " LV_SYMBOL_DOWN,
             sdMounted ? LV_SYMBOL_SD_CARD : "");
    lv_label_set_text(s_lbl_icons, iconBuf);
}

} // namespace UI
} // namespace CydOS
