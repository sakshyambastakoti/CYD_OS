/**
 * @file app_touch_cal.h
 * @brief Interactive Touch Calibration Utility for CYD OS
 */

#pragma once

#include "app_base.h"

namespace CydOS {
namespace Apps {

class TouchCalApp : public AppBase {
public:
    TouchCalApp();
    virtual ~TouchCalApp();

    const char* get_name() const override { return "Calibration"; }
    const char* get_icon() const override { return "\xEF\x85\x9B"; }

    lv_obj_t* create_screen() override;
    void destroy_screen() override;
    void update() override;

    void run_calibration();

private:
    lv_obj_t* m_screen;
    lv_obj_t* m_lbl_status;
    lv_obj_t* m_btn_start;
    lv_obj_t* m_btn_reset;
    bool      m_calibrating;

    static void start_calibration_cb(lv_event_t* e);
    static void reset_calibration_cb(lv_event_t* e);
};

} // namespace Apps
} // namespace CydOS
