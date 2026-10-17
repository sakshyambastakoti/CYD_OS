/**
 * @file app_manager.cpp
 * @brief Application Navigation & Lifecycle Manager implementation
 */

#include "app_manager.h"
#include "home_screen.h"
#include "nav_bar.h"
#include "status_bar.h"
#include "apps/app_base.h"
#include "apps/app_calculator.h"
#include "apps/app_stopwatch.h"
#include "apps/app_torch.h"
#include "apps/app_settings.h"
#include "apps/app_diagnostics.h"
#include "apps/app_touch_cal.h"
#include <Arduino.h>

namespace CydOS {
namespace UI {

static AppID           s_current_id  = APP_HOME;
static Apps::AppBase*  s_active_app  = nullptr;
static lv_obj_t*       s_home_scr    = nullptr;

// Static app instances
static Apps::CalculatorApp   s_app_calc;
static Apps::StopwatchApp    s_app_stopwatch;
static Apps::TorchApp        s_app_torch;
static Apps::SettingsApp     s_app_settings;
static Apps::DiagnosticsApp  s_app_diag;
static Apps::TouchCalApp     s_app_touch_cal;

void app_manager_init() {
    s_current_id = APP_HOME;
    s_active_app = nullptr;
    s_home_scr = home_screen_create();
    lv_scr_load(s_home_scr);
    nav_bar_set_back_visible(false);
    Serial.println("[APP_MGR] Initialized with Home screen loaded");
}

void app_manager_open(AppID app_id) {
    if (app_id == s_current_id || app_id == APP_HOME) {
        app_manager_show_home();
        return;
    }

    // 1. Destroy previous active app screen to reclaim memory
    if (s_active_app != nullptr) {
        s_active_app->destroy_screen();
        s_active_app = nullptr;
    }

    // 2. Select target app instance
    Apps::AppBase* target = nullptr;
    switch (app_id) {
        case APP_CALCULATOR:   target = &s_app_calc; break;
        case APP_STOPWATCH:    target = &s_app_stopwatch; break;
        case APP_TORCH:        target = &s_app_torch; break;
        case APP_SETTINGS:     target = &s_app_settings; break;
        case APP_DIAGNOSTICS:  target = &s_app_diag; break;
        case APP_TOUCH_CAL:    target = &s_app_touch_cal; break;
        default: break;
    }

    if (target == nullptr) {
        return;
    }

    s_active_app = target;
    s_current_id = app_id;

    // 3. Create screen on-demand and load it
    lv_obj_t* scr = s_active_app->create_screen();
    if (scr) {
        lv_scr_load(scr);
    }

    nav_bar_set_back_visible(true);
    Serial.printf("[APP_MGR] Opened app %d (%s) | Free Heap: %uK\n", 
                  app_id, target->get_name(), ESP.getFreeHeap() / 1024);
}

void app_manager_pop() {
    app_manager_show_home();
}

void app_manager_show_home() {
    if (s_active_app != nullptr) {
        s_active_app->destroy_screen();
        s_active_app = nullptr;
    }

    s_current_id = APP_HOME;
    if (s_home_scr) {
        lv_scr_load(s_home_scr);
        home_screen_update();
    }
    nav_bar_set_back_visible(false);
    Serial.printf("[APP_MGR] Returned to Home | Free Heap: %uK\n", ESP.getFreeHeap() / 1024);
}

AppID app_manager_get_current() {
    return s_current_id;
}

void app_manager_update() {
    if (s_current_id == APP_HOME) {
        home_screen_update();
    } else if (s_active_app != nullptr) {
        s_active_app->update();
    }
}

} // namespace UI
} // namespace CydOS
