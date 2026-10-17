/**
 * @file audio_hal.cpp
 * @brief Audio HAL implementation for CYD OS
 * 
 * Safely drives speaker on GPIO 26 using LEDC channel 0.
 * To avoid DC coil saturation and brownouts on USB power, the pin is only
 * attached to LEDC when playing a tone, and tri-stated (INPUT) when idle.
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
    pinMode(PIN_SPEAKER, INPUT); // Default high-impedance safe state (no current draw)
    Serial.println("[AUDIO] Speaker initialized in safe high-impedance mode on GPIO 26");
    return true;
}

void audio_play_tone(uint32_t freq, uint32_t duration_ms) {
    if (s_muted || s_volume == 0 || freq == 0) {
        return;
    }

    // Attach pin and set up tone only when playing
    ledcSetup(AUDIO_LEDC_CHANNEL, freq, AUDIO_LEDC_RESOLUTION);
    ledcAttachPin(PIN_SPEAKER, AUDIO_LEDC_CHANNEL);

    // Scale duty cycle by volume
    uint32_t duty = (s_volume * 128) / 100;
    ledcWrite(AUDIO_LEDC_CHANNEL, duty);

    if (duration_ms > 0) {
        delay(duration_ms);
        // Turn off and detach to prevent continuous DC draw
        ledcWrite(AUDIO_LEDC_CHANNEL, 0);
        ledcDetachPin(PIN_SPEAKER);
        pinMode(PIN_SPEAKER, INPUT);
    }
}

void audio_stop() {
    ledcWrite(AUDIO_LEDC_CHANNEL, 0);
    ledcDetachPin(PIN_SPEAKER);
    pinMode(PIN_SPEAKER, INPUT);
}

void audio_play_click() {
    if (s_muted) return;
    audio_play_tone(2400, 8);
}

void audio_play_alert() {
    if (s_muted) return;
    audio_play_tone(440, 60);
    delay(20);
    audio_play_tone(330, 80);
}

void audio_play_notification() {
    if (s_muted) return;
    audio_play_tone(1046, 30);
    delay(15);
    audio_play_tone(1318, 30);
    delay(15);
    audio_play_tone(1568, 50);
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
