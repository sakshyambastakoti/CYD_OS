/**
 * @file led_hal.cpp
 * @brief RGB LED HAL implementation for CYD OS
 * 
 * Active LOW logic: driving pin LOW turns LED ON, HIGH turns OFF.
 */

#include "led_hal.h"
#include "pin_config.h"
#include <Arduino.h>

namespace CydOS {
namespace HAL {

void led_init() {
    pinMode(PIN_LED_RED,   OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_BLUE,  OUTPUT);
    led_off();
    Serial.println("[LED] RGB LED initialized (Active LOW)");
}

void led_set(bool red, bool green, bool blue) {
    // Active LOW: LOW = ON, HIGH = OFF
    digitalWrite(PIN_LED_RED,   red   ? LOW : HIGH);
    digitalWrite(PIN_LED_GREEN, green ? LOW : HIGH);
    digitalWrite(PIN_LED_BLUE,  blue  ? LOW : HIGH);
}

void led_off()     { led_set(false, false, false); }
void led_red()     { led_set(true,  false, false); }
void led_green()   { led_set(false, true,  false); }
void led_blue()    { led_set(false, false, true);  }
void led_cyan()    { led_set(false, true,  true);  }
void led_magenta() { led_set(true,  false, true);  }
void led_yellow()  { led_set(true,  true,  false); }
void led_white()   { led_set(true,  true,  true);  }

void led_flash(bool red, bool green, bool blue, uint32_t duration_ms) {
    led_set(red, green, blue);
    delay(duration_ms);
    led_off();
}

} // namespace HAL
} // namespace CydOS
