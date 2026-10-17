/**
 * @file storage_service.h
 * @brief Persistent Settings Service using ESP32 NVS (Preferences)
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <WString.h>

namespace CydOS {
namespace Services {

struct SystemSettings {
    uint8_t  brightness;
    uint32_t screen_timeout_sec;
    bool     auto_brightness;
    uint8_t  volume;
    bool     audio_muted;
    bool     clock_24h;
    bool     touch_calibrated;
    uint16_t touch_cal[8];
    char     wifi_ssid[33];
    char     wifi_pass[65];
};

/**
 * @brief Initialize storage service and load persistent settings
 */
bool storage_init();

/**
 * @brief Get current in-memory system settings
 */
SystemSettings& storage_get_settings();

/**
 * @brief Save all modified settings to NVS flash
 */
bool storage_save();

/**
 * @brief Reset settings to factory defaults
 */
void storage_reset_defaults();

} // namespace Services
} // namespace CydOS
