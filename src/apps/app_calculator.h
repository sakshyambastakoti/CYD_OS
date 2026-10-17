/**
 * @file app_calculator.h
 * @brief Smartphone Calculator Application for CYD OS
 */

#pragma once

#include "app_base.h"

namespace CydOS {
namespace Apps {

class CalculatorApp : public AppBase {
public:
    CalculatorApp();
    virtual ~CalculatorApp();

    const char* get_name() const override { return "Calculator"; }
    const char* get_icon() const override { return "\xEF\x87\xAC"; } // Calculator icon or text

    lv_obj_t* create_screen() override;
    void destroy_screen() override;
    void update() override {}

private:
    lv_obj_t* m_screen;
    lv_obj_t* m_lbl_display;
    char      m_display_buf[32];
    double    m_accum;
    double    m_operand;
    char      m_pending_op;
    bool      m_new_number;

    void handle_input(const char* key);
    void update_display();
    static void btn_click_cb(lv_event_t* e);
};

} // namespace Apps
} // namespace CydOS
