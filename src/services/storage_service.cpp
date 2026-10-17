/**
 * @file storage_service.cpp
 * @brief Persistent Settings Service implementation using Preferences
 */

#include "storage_service.h"
#include "app_config.h"
#include <Preferences.h>
#include <string.h>

namespace CydOS {
namespace Services {

static Preferences s_prefs;
static SystemSettings s_settings;

void storage_reset_defaults() {
    s_settings.brightness         = DEFAULT_BRIGHTNESS;
    s_settings.screen_timeout_sec = 30;
    s_settings.auto_brightness    = false;
    s_settings.volume             = 80;
    s_settings.audio_muted        = false;
    s_settings.clock_24h          = true;
    s_settings.touch_calibrated   = false;
    memset(s_settings.touch_cal, 0, sizeof(s_settings.touch_cal));
    s_settings.wifi_ssid[0]       = '\0';
    s_settings.wifi_pass[0]       = '\0';
}

bool storage_init() {
    storage_reset_defaults();

    if (!s_prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("[STORAGE] Warning: Failed to open NVS namespace, using defaults");
        return false;
    }

    s_settings.brightness         = s_prefs.getUChar("bright", DEFAULT_BRIGHTNESS);
    if (s_settings.brightness < 60) s_settings.brightness = DEFAULT_BRIGHTNESS;

    s_settings.screen_timeout_sec = s_prefs.getUInt("timeout", 60);
    if (s_settings.screen_timeout_sec != 0 && s_settings.screen_timeout_sec < 15) {
        s_settings.screen_timeout_sec = 60;
    }

    s_settings.auto_brightness    = s_prefs.getBool("autobright", false);
    s_settings.volume             = s_prefs.getUChar("volume", 80);
    s_settings.audio_muted        = s_prefs.getBool("muted", false);
    s_settings.clock_24h          = s_prefs.getBool("clk24", true);
    s_settings.touch_calibrated   = s_prefs.getBool("cal_ok", false);

    if (s_settings.touch_calibrated) {
        size_t len = s_prefs.getBytes("cal_data", s_settings.touch_cal, sizeof(s_settings.touch_cal));
        bool valid = (len == sizeof(s_settings.touch_cal));
        if (valid) {
            for (int i = 0; i < 8; i++) {
                if (s_settings.touch_cal[i] < 100 || s_settings.touch_cal[i] > 4000) {
                    valid = false;
                    break;
                }
            }
        }
        if (!valid) {
            Serial.println("[STORAGE] Stored touch calibration was invalid or zeroed. Discarding.");
            s_settings.touch_calibrated = false;
            memset(s_settings.touch_cal, 0, sizeof(s_settings.touch_cal));
            s_prefs.putBool("cal_ok", false);
            s_prefs.remove("cal_data");
        }
    }

    String ssid = s_prefs.getString("wifi_ssid", "");
    String pass = s_prefs.getString("wifi_pass", "");
    strncpy(s_settings.wifi_ssid, ssid.c_str(), sizeof(s_settings.wifi_ssid) - 1);
    strncpy(s_settings.wifi_pass, pass.c_str(), sizeof(s_settings.wifi_pass) - 1);

    s_prefs.end();

    Serial.printf("[STORAGE] Settings loaded (Brightness: %d, Timeout: %ds, Vol: %d)\n",
                  s_settings.brightness, s_settings.screen_timeout_sec, s_settings.volume);
    return true;
}

SystemSettings& storage_get_settings() {
    return s_settings;
}

bool storage_save() {
    if (!s_prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("[STORAGE] ERROR: Failed to open NVS for writing!");
        return false;
    }

    s_prefs.putUChar("bright", s_settings.brightness);
    s_prefs.putUInt("timeout", s_settings.screen_timeout_sec);
    s_prefs.putBool("autobright", s_settings.auto_brightness);
    s_prefs.putUChar("volume", s_settings.volume);
    s_prefs.putBool("muted", s_settings.audio_muted);
    s_prefs.putBool("clk24", s_settings.clock_24h);
    s_prefs.putBool("cal_ok", s_settings.touch_calibrated);

    if (s_settings.touch_calibrated) {
        s_prefs.putBytes("cal_data", s_settings.touch_cal, sizeof(s_settings.touch_cal));
    }

    s_prefs.putString("wifi_ssid", s_settings.wifi_ssid);
    s_prefs.putString("wifi_pass", s_settings.wifi_pass);

    s_prefs.end();
    Serial.println("[STORAGE] Settings successfully saved to NVS");
    return true;
}

} // namespace Services
} // namespace CydOS
