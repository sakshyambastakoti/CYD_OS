/**
 * @file app_touch_cal.h
 * @brief Interactive Touch Calibration & Accuracy Verification Utility for CYD OS
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
    void open_touch_test();
    void close_touch_test();

private:
    lv_obj_t* m_screen;
    lv_obj_t* m_lbl_status;
    lv_obj_t* m_btn_start;
    lv_obj_t* m_btn_test;
    lv_obj_t* m_btn_reset;
    bool      m_calibrating;

    // Live test canvas elements
    lv_obj_t* m_test_overlay;
    lv_obj_t* m_lbl_test_coords;
    lv_obj_t* m_cursor_dot;
    lv_obj_t* m_targets[5]; // 0: TL, 1: TR, 2: Center, 3: BL, 4: BR
    bool      m_target_hit[5];

    static void start_calibration_cb(lv_event_t* e);
    static void open_test_cb(lv_event_t* e);
    static void close_test_cb(lv_event_t* e);
    static void reset_calibration_cb(lv_event_t* e);
    static void target_click_cb(lv_event_t* e);
};

} // namespace Apps
} // namespace CydOS
