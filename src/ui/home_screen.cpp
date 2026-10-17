/**
 * @file home_screen.cpp
 * @brief Smartphone Home Screen & App Launcher implementation
 */

#include "home_screen.h"
#include "app_manager.h"
#include "quick_settings.h"
#include "theme.h"
#include "services/time_service.h"
#include "hal/audio_hal.h"
#include "hal/sd_hal.h"
#include <Arduino.h>

namespace CydOS {
namespace UI {

static lv_obj_t* s_home_screen = nullptr;
static lv_obj_t* s_lbl_clock   = nullptr;
static lv_obj_t* s_lbl_date    = nullptr;
static lv_obj_t* s_lbl_status  = nullptr;

struct AppLauncherItem {
    const char* name;
    const char* symbol;
    uint32_t    color_hex;
    AppID       app_id;
};

static const AppLauncherItem s_apps[] = {
    {"Calc",     LV_SYMBOL_EDIT,      0x00ADB5, APP_CALCULATOR},
    {"Timer",    LV_SYMBOL_PLAY,      0x4ECCA3, APP_STOPWATCH},
    {"Torch",    LV_SYMBOL_EYE_OPEN,  0xFF8A5B, APP_TORCH},
    {"Settings", LV_SYMBOL_SETTINGS,  0x8F94A6, APP_SETTINGS},
    {"Diag",     LV_SYMBOL_REFRESH,   0x9B59B6, APP_DIAGNOSTICS},
    {"TouchCal", LV_SYMBOL_PLUS,      0x2ECC71, APP_TOUCH_CAL},
    {"Storage",  LV_SYMBOL_SD_CARD,   0xF39C12, APP_NONE},
    {"Controls", LV_SYMBOL_DOWN,      0x3498DB, APP_NONE}
};

static void app_btn_click_event(lv_event_t* e) {
    HAL::audio_play_click();
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    if (idx >= sizeof(s_apps) / sizeof(s_apps[0])) return;

    AppID id = s_apps[idx].app_id;
    if (id != APP_NONE) {
        app_manager_open(id);
    } else {
        if (strcmp(s_apps[idx].name, "Controls") == 0) {
            quick_settings_show();
        } else if (strcmp(s_apps[idx].name, "Storage") == 0) {
            // Flash a quick SD notification chime
            HAL::audio_play_notification();
        }
    }
}

lv_obj_t* home_screen_create() {
    if (s_home_screen) {
        return s_home_screen;
    }

    s_home_screen = lv_obj_create(nullptr);
    lv_obj_add_style(s_home_screen, &style_screen_bg, 0);
    lv_obj_set_size(s_home_screen, 240, 320);
    lv_obj_set_pos(s_home_screen, 0, 0);
    lv_obj_clear_flag(s_home_screen, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Clock & Date Widget Card (below 22px status bar)
    lv_obj_t* clock_card = lv_obj_create(s_home_screen);
    lv_obj_add_style(clock_card, &style_card, 0);
    lv_obj_set_size(clock_card, 224, 76);
    lv_obj_set_pos(clock_card, 8, 26);
    lv_obj_clear_flag(clock_card, LV_OBJ_FLAG_SCROLLABLE);

    s_lbl_clock = lv_label_create(clock_card);
    lv_obj_set_style_text_color(s_lbl_clock, THEME_COLOR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(s_lbl_clock, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(s_lbl_clock, 6, 2);
    lv_label_set_text(s_lbl_clock, "12:00");

    s_lbl_date = lv_label_create(clock_card);
    lv_obj_add_style(s_lbl_date, &style_subtitle, 0);
    lv_obj_set_pos(s_lbl_date, 8, 38);
    lv_label_set_text(s_lbl_date, "Saturday, Oct 17");

    s_lbl_status = lv_label_create(clock_card);
    lv_obj_add_style(s_lbl_status, &style_pill, 0);
    lv_obj_align(s_lbl_status, LV_ALIGN_TOP_RIGHT, 0, 4);
    lv_label_set_text(s_lbl_status, "CYD OS");

    // 2. Apps Launcher Grid: 2 rows of 4 columns
    uint16_t start_x = 10;
    uint16_t start_y = 110;
    uint16_t col_w = 54;
    uint16_t row_h = 84;

    for (size_t i = 0; i < sizeof(s_apps) / sizeof(s_apps[0]); i++) {
        uint8_t row = i / 4;
        uint8_t col = i % 4;

        int16_t x = start_x + col * col_w;
        int16_t y = start_y + row * row_h;

        // Icon Button
        lv_obj_t* btn = lv_btn_create(s_home_screen);
        lv_obj_add_style(btn, &style_btn, 0);
        lv_obj_set_size(btn, 44, 44);
        lv_obj_set_pos(btn, x + 2, y);
        lv_obj_set_style_bg_color(btn, lv_color_hex(s_apps[i].color_hex), 0);
        lv_obj_set_style_radius(btn, 12, 0);
        lv_obj_add_event_cb(btn, app_btn_click_event, LV_EVENT_CLICKED, (void*)(uintptr_t)i);

        lv_obj_t* icon = lv_label_create(btn);
        lv_label_set_text(icon, s_apps[i].symbol);
        lv_obj_set_style_text_color(icon, lv_color_white(), 0);
        lv_obj_set_style_text_font(icon, &lv_font_montserrat_14, 0);
        lv_obj_center(icon);

        // Name Label
        lv_obj_t* lbl = lv_label_create(s_home_screen);
        lv_obj_add_style(lbl, &style_subtitle, 0);
        lv_label_set_text(lbl, s_apps[i].name);
        lv_obj_set_pos(lbl, x - 2, y + 48);
        lv_obj_set_size(lbl, 52, 18);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    }

    home_screen_update();
    return s_home_screen;
}

void home_screen_update() {
    if (s_lbl_clock == nullptr || s_lbl_date == nullptr) return;

    char timeBuf[16];
    Services::time_get_formatted(timeBuf, sizeof(timeBuf), false);
    lv_label_set_text(s_lbl_clock, timeBuf);

    char dateBuf[32];
    Services::time_get_date_formatted(dateBuf, sizeof(dateBuf));
    lv_label_set_text(s_lbl_date, dateBuf);

    if (s_lbl_status) {
        lv_label_set_text_fmt(s_lbl_status, "%uK free", ESP.getFreeHeap() / 1024);
    }
}

} // namespace UI
} // namespace CydOS
