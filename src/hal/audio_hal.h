/**
 * @file audio_hal.h
 * @brief Audio Hardware Abstraction Layer for CYD OS
 * 
 * Drives the onboard speaker amplifier on GPIO 26 using ESP32 LEDC PWM.
 * Provides system feedback sounds (tap clicks, alerts, notifications, alarms).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

namespace CydOS {
namespace HAL {

/**
 * @brief Initialize audio hardware (LEDC channel 0 on GPIO 26)
 */
bool audio_init();

/**
 * @brief Play a pure tone
 * @param freq Frequency in Hz
 * @param duration_ms Duration in milliseconds (non-blocking if duration > 0, blocking delay handled or timed)
 */
void audio_play_tone(uint32_t freq, uint32_t duration_ms);

/**
 * @brief Stop current sound immediately
 */
void audio_stop();

/**
 * @brief Play a subtle UI tap click sound (short click)
 */
void audio_play_click();

/**
 * @brief Play an alert / error beep
 */
void audio_play_alert();

/**
 * @brief Play a notification chime
 */
void audio_play_notification();

/**
 * @brief Set system audio volume (0-100)
 */
void audio_set_volume(uint8_t volume);

/**
 * @brief Get current system volume (0-100)
 */
uint8_t audio_get_volume();

/**
 * @brief Mute or unmute audio
 */
void audio_set_muted(bool muted);

/**
 * @brief Check if audio is currently muted
 */
bool audio_is_muted();

} // namespace HAL
} // namespace CydOS
