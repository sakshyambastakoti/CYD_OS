/**
 * @file quick_settings.cpp
 * @brief Pull-down Quick Settings Control Center implementation
 */

#include "quick_settings.h"
#include "theme.h"
#include "hal/display_hal.h"
#include "hal/audio_hal.h"
#include "hal/led_hal.h"
#include "services/storage_service.h"
#include <Arduino.h>

namespace CydOS {
namespace UI {

static lv_obj_t* s_shade_overlay    = nullptr;
static lv_obj_t* s_slider_bright    = nullptr;
static lv_obj_t* s_slider_vol       = nullptr;
static lv_obj_t* s_btn_torch        = nullptr;
static lv_obj_t* s_lbl_torch        = nullptr;
static bool      s_torch_state      = false;

static void close_click_event(lv_event_t* e) {
    HAL::audio_play_click();
    quick_settings_hide();
}

static void brightness_slider_event(lv_event_t* e) {
    int32_t val = lv_slider_get_value(s_slider_bright);
    HAL::display_set_brightness((uint8_t)val);
    Services::storage_get_settings().brightness = (uint8_t)val;
}

static void volume_slider_event(lv_event_t* e) {
    int32_t val = lv_slider_get_value(s_slider_vol);
    HAL::audio_set_volume((uint8_t)val);
    Services::storage_get_settings().volume = (uint8_t)val;
    HAL::audio_play_click();
}

static void torch_toggle_event(lv_event_t* e) {
    s_torch_state = !s_torch_state;
    if (s_torch_state) {
        HAL::led_set_rgb(true, true, true); // White torch
        lv_label_set_text(s_lbl_torch, "Torch ON");
        lv_obj_set_style_bg_color(s_btn_torch, THEME_COLOR_PRIMARY, 0);
    } else {
        HAL::led_set_rgb(false, false, false);
        lv_label_set_text(s_lbl_torch, "Torch OFF");
        lv_obj_set_style_bg_color(s_btn_torch, THEME_COLOR_SURFACE_ALT, 0);
    }
    HAL::audio_play_click();
}

static void reboot_click_event(lv_event_t* e) {
    HAL::audio_play_alert();
    delay(200);
    ESP.restart();
}

void quick_settings_init() {
    lv_obj_t* top = lv_layer_top();

    // Fullscreen shade container
    s_shade_overlay = lv_obj_create(top);
    lv_obj_remove_style_all(s_shade_overlay);
    lv_obj_set_size(s_shade_overlay, 240, 320);
    lv_obj_set_pos(s_shade_overlay, 0, 0);
    lv_obj_set_style_bg_color(s_shade_overlay, lv_color_hex(0x0E0F18), 0);
    lv_obj_set_style_bg_opa(s_shade_overlay, LV_OPA_90, 0);
    lv_obj_clear_flag(s_shade_overlay, LV_OBJ_FLAG_SCROLLABLE);

    // Header card
    lv_obj_t* header = lv_obj_create(s_shade_overlay);
    lv_obj_add_style(header, &style_card, 0);
    lv_obj_set_size(header, 224, 38);
    lv_obj_set_pos(header, 8, 8);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lbl_title = lv_label_create(header);
    lv_label_set_text(lbl_title, LV_SYMBOL_SETTINGS " Quick Controls");
    lv_obj_add_style(lbl_title, &style_title, 0);
    lv_obj_align(lbl_title, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t* btn_close = lv_btn_create(header);
    lv_obj_add_style(btn_close, &style_btn, 0);
    lv_obj_set_size(btn_close, 30, 26);
    lv_obj_align(btn_close, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(btn_close, close_click_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* lbl_x = lv_label_create(btn_close);
    lv_label_set_text(lbl_x, LV_SYMBOL_CLOSE);
    lv_obj_center(lbl_x);

    // Controls Card
    lv_obj_t* card = lv_obj_create(s_shade_overlay);
    lv_obj_add_style(card, &style_card, 0);
    lv_obj_set_size(card, 224, 250);
    lv_obj_set_pos(card, 8, 52);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Brightness
    lv_obj_t* lbl_b = lv_label_create(card);
    lv_label_set_text(lbl_b, LV_SYMBOL_EYE_OPEN " Screen Brightness");
    lv_obj_add_style(lbl_b, &style_subtitle, 0);
    lv_obj_set_pos(lbl_b, 0, 4);

    s_slider_bright = lv_slider_create(card);
    lv_slider_set_range(s_slider_bright, 15, 255);
    lv_slider_set_value(s_slider_bright, Services::storage_get_settings().brightness, LV_ANIM_OFF);
    lv_obj_set_size(s_slider_bright, 204, 16);
    lv_obj_set_pos(s_slider_bright, 2, 26);
    lv_obj_set_style_bg_color(s_slider_bright, THEME_COLOR_PRIMARY, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(s_slider_bright, THEME_COLOR_TEXT_MAIN, LV_PART_KNOB);
    lv_obj_add_event_cb(s_slider_bright, brightness_slider_event, LV_EVENT_VALUE_CHANGED, nullptr);

    // 2. Volume
    lv_obj_t* lbl_v = lv_label_create(card);
    lv_label_set_text(lbl_v, LV_SYMBOL_VOLUME_MAX " Audio Volume");
    lv_obj_add_style(lbl_v, &style_subtitle, 0);
    lv_obj_set_pos(lbl_v, 0, 56);

    s_slider_vol = lv_slider_create(card);
    lv_slider_set_range(s_slider_vol, 0, 100);
    lv_slider_set_value(s_slider_vol, Services::storage_get_settings().volume, LV_ANIM_OFF);
    lv_obj_set_size(s_slider_vol, 204, 16);
    lv_obj_set_pos(s_slider_vol, 2, 78);
    lv_obj_set_style_bg_color(s_slider_vol, THEME_COLOR_SECONDARY, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(s_slider_vol, THEME_COLOR_TEXT_MAIN, LV_PART_KNOB);
    lv_obj_add_event_cb(s_slider_vol, volume_slider_event, LV_EVENT_VALUE_CHANGED, nullptr);

    // 3. Quick Action Buttons
    s_btn_torch = lv_btn_create(card);
    lv_obj_add_style(s_btn_torch, &style_btn, 0);
    lv_obj_set_size(s_btn_torch, 98, 38);
    lv_obj_set_pos(s_btn_torch, 2, 116);
    lv_obj_add_event_cb(s_btn_torch, torch_toggle_event, LV_EVENT_CLICKED, nullptr);
    s_lbl_torch = lv_label_create(s_btn_torch);
    lv_label_set_text(s_lbl_torch, "Torch OFF");
    lv_obj_add_style(s_lbl_torch, &style_subtitle, 0);
    lv_obj_center(s_lbl_torch);

    lv_obj_t* btn_reboot = lv_btn_create(card);
    lv_obj_add_style(btn_reboot, &style_btn, 0);
    lv_obj_set_size(btn_reboot, 98, 38);
    lv_obj_set_pos(btn_reboot, 108, 116);
    lv_obj_set_style_bg_color(btn_reboot, THEME_COLOR_DANGER, 0);
    lv_obj_add_event_cb(btn_reboot, reboot_click_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* lbl_rb = lv_label_create(btn_reboot);
    lv_label_set_text(lbl_rb, LV_SYMBOL_REFRESH " Reboot");
    lv_obj_add_style(lbl_rb, &style_subtitle, 0);
    lv_obj_center(lbl_rb);

    // System stats snippet at bottom of quick controls
    lv_obj_t* lbl_stats = lv_label_create(card);
    lv_obj_set_pos(lbl_stats, 2, 180);
    lv_obj_add_style(lbl_stats, &style_subtitle, 0);
    lv_label_set_text_fmt(lbl_stats, "CYD OS v0.2 | Free Heap: %uK", ESP.getFreeHeap() / 1024);

    // Start hidden
    lv_obj_add_flag(s_shade_overlay, LV_OBJ_FLAG_HIDDEN);
    Serial.println("[UI] Quick Settings overlay initialized");
}

void quick_settings_show() {
    if (s_shade_overlay == nullptr) return;
    lv_slider_set_value(s_slider_bright, Services::storage_get_settings().brightness, LV_ANIM_OFF);
    lv_slider_set_value(s_slider_vol, Services::storage_get_settings().volume, LV_ANIM_OFF);
    lv_obj_clear_flag(s_shade_overlay, LV_OBJ_FLAG_HIDDEN);
}

void quick_settings_hide() {
    if (s_shade_overlay == nullptr) return;
    lv_obj_add_flag(s_shade_overlay, LV_OBJ_FLAG_HIDDEN);
    Services::storage_save(); // Save any adjusted slider settings
}

void quick_settings_toggle() {
    if (quick_settings_is_visible()) {
        quick_settings_hide();
    } else {
        quick_settings_show();
    }
}

bool quick_settings_is_visible() {
    if (s_shade_overlay == nullptr) return false;
    return !lv_obj_has_flag(s_shade_overlay, LV_OBJ_FLAG_HIDDEN);
}

} // namespace UI
} // namespace CydOS
