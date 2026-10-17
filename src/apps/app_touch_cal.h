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

private:
    lv_obj_t* m_screen;
    lv_obj_t* m_target;
    lv_obj_t* m_lbl_step;

    uint8_t   m_step; // 0 = TL, 1 = TR, 2 = BR, 3 = BL, 4 = Done
    uint32_t  m_last_step_time;
    bool      m_waiting_release;
    int16_t   m_samples[4][2]; // [step][0=X, 1=Y]

    void set_target_pos(int16_t x, int16_t y);
    void finish_calibration();
};

} // namespace Apps
} // namespace CydOS
