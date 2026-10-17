/**
 * @file audio_hal.cpp
 * @brief Audio HAL implementation for CYD OS
 */

#include "audio_hal.h"
#include "pin_config.h"
#include <Arduino.h>

#define AUDIO_LEDC_CHANNEL     0
#define AUDIO_LEDC_RESOLUTION  8

namespace CydOS {
namespace HAL {

static uint8_t s_volume = 80;
static bool    s_muted  = false;

bool audio_init() {
    ledcSetup(AUDIO_LEDC_CHANNEL, 1000, AUDIO_LEDC_RESOLUTION);
    ledcAttachPin(PIN_SPEAKER, AUDIO_LEDC_CHANNEL);
    ledcWrite(AUDIO_LEDC_CHANNEL, 0); // Start silent
    Serial.println("[AUDIO] Speaker initialized on GPIO 26 (LEDC Channel 0)");
    return true;
}

void audio_play_tone(uint32_t freq, uint32_t duration_ms) {
    if (s_muted || s_volume == 0 || freq == 0) {
        return;
    }

    ledcWriteTone(AUDIO_LEDC_CHANNEL, freq);
    // Scale duty cycle by volume
    uint32_t duty = (s_volume * 128) / 100;
    ledcWrite(AUDIO_LEDC_CHANNEL, duty);

    if (duration_ms > 0) {
        delay(duration_ms);
        ledcWrite(AUDIO_LEDC_CHANNEL, 0);
    }
}

void audio_stop() {
    ledcWrite(AUDIO_LEDC_CHANNEL, 0);
}

void audio_play_click() {
    if (s_muted) return;
    // High-pitch crisp 2.4kHz click for 8ms
    audio_play_tone(2400, 8);
}

void audio_play_alert() {
    if (s_muted) return;
    audio_play_tone(440, 60);
    delay(30);
    audio_play_tone(330, 90);
}

void audio_play_notification() {
    if (s_muted) return;
    // Pleasant rising chime: C6 -> E6 -> G6
    audio_play_tone(1046, 35);
    delay(10);
    audio_play_tone(1318, 35);
    delay(10);
    audio_play_tone(1568, 60);
}

void audio_set_volume(uint8_t volume) {
    if (volume > 100) volume = 100;
    s_volume = volume;
}

uint8_t audio_get_volume() {
    return s_volume;
}

void audio_set_muted(bool muted) {
    s_muted = muted;
    if (muted) {
        audio_stop();
    }
}

bool audio_is_muted() {
    return s_muted;
}

} // namespace HAL
} // namespace CydOS
