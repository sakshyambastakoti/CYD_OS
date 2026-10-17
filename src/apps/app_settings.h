/**
 * @file app_settings.h
 * @brief System Settings Application for CYD OS
 */

#pragma once

#include "app_base.h"

namespace CydOS {
namespace Apps {

class SettingsApp : public AppBase {
public:
    SettingsApp();
    virtual ~SettingsApp();

    const char* get_name() const override { return "Settings"; }
    const char* get_icon() const override { return "\xEF\x80\x93"; } // Gear icon

    lv_obj_t* create_screen() override;
    void destroy_screen() override;
    void update() override {}

private:
    lv_obj_t* m_screen;
    lv_obj_t* m_slider_bright;
    lv_obj_t* m_slider_vol;
    lv_obj_t* m_dd_timeout;

    static void bright_slider_cb(lv_event_t* e);
    static void vol_slider_cb(lv_event_t* e);
    static void timeout_dd_cb(lv_event_t* e);
    static void touch_cal_btn_cb(lv_event_t* e);
    static void save_btn_cb(lv_event_t* e);
};

} // namespace Apps
} // namespace CydOS
