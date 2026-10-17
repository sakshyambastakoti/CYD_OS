/**
 * @file app_torch.h
 * @brief Torch & Flashlight Application for CYD OS
 */

#pragma once

#include "app_base.h"

namespace CydOS {
namespace Apps {

class TorchApp : public AppBase {
public:
    TorchApp();
    virtual ~TorchApp();

    const char* get_name() const override { return "Flashlight"; }
    const char* get_icon() const override { return "\xEF\x83\xAB"; } // Lightbulb icon

    lv_obj_t* create_screen() override;
    void destroy_screen() override;
    void update() override;

private:
    lv_obj_t* m_screen;
    lv_obj_t* m_btn_screen_torch;
    lv_obj_t* m_btn_led_torch;
    lv_obj_t* m_btn_sos;

    bool      m_screen_torch_on;
    bool      m_led_torch_on;
    bool      m_sos_on;
    uint8_t   m_orig_brightness;
    uint32_t  m_sos_last_toggle;
    bool      m_sos_state;

    static void screen_torch_cb(lv_event_t* e);
    static void led_torch_cb(lv_event_t* e);
    static void sos_cb(lv_event_t* e);
};

} // namespace Apps
} // namespace CydOS
