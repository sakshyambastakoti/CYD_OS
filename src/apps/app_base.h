/**
 * @file app_base.h
 * @brief Base interface for all CYD OS applications
 */

#pragma once

#include <lvgl.h>

namespace CydOS {
namespace Apps {

class AppBase {
public:
    virtual ~AppBase() {}
    virtual const char* get_name() const = 0;
    virtual const char* get_icon() const = 0;
    virtual lv_obj_t* create_screen() = 0;
    virtual void destroy_screen() = 0;
    virtual void update() {}
};

} // namespace Apps
} // namespace CydOS
