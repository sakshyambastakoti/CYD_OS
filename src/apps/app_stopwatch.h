/**
 * @file app_stopwatch.h
 * @brief Stopwatch & Timer Application for CYD OS
 */

#pragma once

#include "app_base.h"

namespace CydOS {
namespace Apps {

class StopwatchApp : public AppBase {
public:
    StopwatchApp();
    virtual ~StopwatchApp();

    const char* get_name() const override { return "Stopwatch"; }
    const char* get_icon() const override { return "\xEF\x80\x97"; } // Clock/timer icon

    lv_obj_t* create_screen() override;
    void destroy_screen() override;
    void update() override;

private:
    lv_obj_t* m_screen;
    lv_obj_t* m_lbl_time;
    lv_obj_t* m_btn_start_stop;
    lv_obj_t* m_lbl_start_stop;
    lv_obj_t* m_lbl_laps;

    bool      m_running;
    uint32_t  m_start_time;
    uint32_t  m_elapsed_time;
    uint32_t  m_last_lap_time;
    uint8_t   m_lap_count;
    char      m_laps_text[128];

    static void start_stop_cb(lv_event_t* e);
    static void reset_lap_cb(lv_event_t* e);
};

} // namespace Apps
} // namespace CydOS
