/**
 * @file app_diagnostics.h
 * @brief Hardware Diagnostics Application for CYD OS
 */

#pragma once

#include "app_base.h"

namespace CydOS {
namespace Apps {

class DiagnosticsApp : public AppBase {
public:
    DiagnosticsApp();
    virtual ~DiagnosticsApp();

    const char* get_name() const override { return "Diagnostics"; }
    const char* get_icon() const override { return "\xEF\x82\x80"; } // Bar chart / activity icon

    lv_obj_t* create_screen() override;
    void destroy_screen() override;
    void update() override;

private:
    lv_obj_t* m_screen;
    lv_obj_t* m_lbl_heap;
    lv_obj_t* m_lbl_ldr;
    lv_obj_t* m_lbl_sd;
    lv_obj_t* m_lbl_touch;
    lv_obj_t* m_lbl_uptime;
};

} // namespace Apps
} // namespace CydOS
