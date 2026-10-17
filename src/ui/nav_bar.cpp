/**
 * @file nav_bar.cpp
 * @brief Bottom Navigation Bar implementation
 */

#include "nav_bar.h"
#include "app_manager.h"
#include "quick_settings.h"
#include "theme.h"
#include "hal/audio_hal.h"
#include <Arduino.h>

namespace CydOS {
namespace UI {

static lv_obj_t* s_nav_cont    = nullptr;
static lv_obj_t* s_btn_back    = nullptr;
static lv_obj_t* s_btn_home    = nullptr;
static lv_obj_t* s_btn_quick   = nullptr;

static void nav_back_event(lv_event_t* e) {
    HAL::audio_play_click();
    app_manager_pop();
}

static void nav_home_event(lv_event_t* e) {
    HAL::audio_play_click();
    app_manager_show_home();
}

static void nav_quick_event(lv_event_t* e) {
    HAL::audio_play_click();
    quick_settings_toggle();
}

void nav_bar_init() {
    lv_obj_t* top = lv_layer_top();

    // Container: 240 x 24 at y = 296
    s_nav_cont = lv_obj_create(top);
    lv_obj_remove_style_all(s_nav_cont);
    lv_obj_set_size(s_nav_cont, 240, 24);
    lv_obj_set_pos(s_nav_cont, 0, 296);
    lv_obj_set_style_bg_color(s_nav_cont, lv_color_hex(0x0C0D14), 0);
    lv_obj_set_style_bg_opa(s_nav_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_nav_cont, 1, 0);
    lv_obj_set_style_border_color(s_nav_cont, THEME_COLOR_BORDER, 0);
    lv_obj_set_style_border_side(s_nav_cont, LV_BORDER_SIDE_TOP, 0);
    lv_obj_clear_flag(s_nav_cont, LV_OBJ_FLAG_SCROLLABLE);

    // Flex layout: space evenly
    lv_obj_set_flex_flow(s_nav_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(s_nav_cont, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 1. Back button
    s_btn_back = lv_btn_create(s_nav_cont);
    lv_obj_remove_style_all(s_btn_back);
    lv_obj_set_size(s_btn_back, 64, 22);
    lv_obj_add_event_cb(s_btn_back, nav_back_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* lbl_b = lv_label_create(s_btn_back);
    lv_label_set_text(lbl_b, LV_SYMBOL_LEFT " Back");
    lv_obj_set_style_text_color(lbl_b, THEME_COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(lbl_b, &lv_font_montserrat_12, 0);
    lv_obj_center(lbl_b);

    // 2. Home button
    s_btn_home = lv_btn_create(s_nav_cont);
    lv_obj_remove_style_all(s_btn_home);
    lv_obj_set_size(s_btn_home, 64, 22);
    lv_obj_add_event_cb(s_btn_home, nav_home_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* lbl_h = lv_label_create(s_btn_home);
    lv_label_set_text(lbl_h, LV_SYMBOL_HOME);
    lv_obj_set_style_text_color(lbl_h, THEME_COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_h, &lv_font_montserrat_14, 0);
    lv_obj_center(lbl_h);

    // 3. Quick / Menu button
    s_btn_quick = lv_btn_create(s_nav_cont);
    lv_obj_remove_style_all(s_btn_quick);
    lv_obj_set_size(s_btn_quick, 64, 22);
    lv_obj_add_event_cb(s_btn_quick, nav_quick_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* lbl_q = lv_label_create(s_btn_quick);
    lv_label_set_text(lbl_q, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_color(lbl_q, THEME_COLOR_TEXT_MUTED, 0);
    lv_obj_set_style_text_font(lbl_q, &lv_font_montserrat_12, 0);
    lv_obj_center(lbl_q);

    Serial.println("[UI] Bottom navigation bar mounted");
}

void nav_bar_set_back_visible(bool visible) {
    if (s_btn_back == nullptr) return;
    if (visible) {
        lv_obj_clear_flag(s_btn_back, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_btn_back, LV_OBJ_FLAG_HIDDEN);
    }
}

} // namespace UI
} // namespace CydOS
