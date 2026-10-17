/**
 * @file time_service.cpp
 * @brief System Time & Clock Service implementation
 */

#include "time_service.h"
#include "storage_service.h"
#include "core/event_bus.h"
#include <Arduino.h>
#include <stdio.h>

namespace CydOS {
namespace Services {

static uint32_t s_base_epoch = 1776426000; // Simulated reasonable base time
static uint32_t s_base_millis = 0;
static uint32_t s_last_tick_sec = 0;

static const char* const s_days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char* const s_months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void time_init() {
    s_base_millis = millis();
    s_last_tick_sec = s_base_millis / 1000;
}

static uint32_t get_current_epoch() {
    uint32_t elapsed_sec = (millis() - s_base_millis) / 1000;
    return s_base_epoch + elapsed_sec;
}

void time_update() {
    uint32_t current_sec = millis() / 1000;
    if (current_sec != s_last_tick_sec) {
        s_last_tick_sec = current_sec;
        Core::event_bus_publish_simple(Core::EVENT_TICK_1S, current_sec);
    }
}

void time_set(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, uint16_t year) {
    // Basic calculation for hour/min/sec
    uint32_t today_sec = (uint32_t)hour * 3600 + (uint32_t)minute * 60 + second;
    s_base_epoch = today_sec;
    s_base_millis = millis();
}

uint8_t time_get_hour() {
    uint32_t sec = get_current_epoch() % 86400;
    return (uint8_t)(sec / 3600);
}

uint8_t time_get_minute() {
    uint32_t sec = get_current_epoch() % 86400;
    return (uint8_t)((sec % 3600) / 60);
}

uint8_t time_get_second() {
    uint32_t sec = get_current_epoch() % 86400;
    return (uint8_t)(sec % 60);
}

void time_get_formatted(char* buf, size_t maxLen, bool includeSeconds) {
    uint8_t h = time_get_hour();
    uint8_t m = time_get_minute();
    uint8_t s = time_get_second();

    bool is24h = storage_get_settings().clock_24h;

    if (is24h) {
        if (includeSeconds) {
            snprintf(buf, maxLen, "%02d:%02d:%02d", h, m, s);
        } else {
            snprintf(buf, maxLen, "%02d:%02d", h, m);
        }
    } else {
        const char* ampm = (h >= 12) ? "PM" : "AM";
        uint8_t h12 = h % 12;
        if (h12 == 0) h12 = 12;
        if (includeSeconds) {
            snprintf(buf, maxLen, "%02d:%02d:%02d %s", h12, m, s, ampm);
        } else {
            snprintf(buf, maxLen, "%02d:%02d %s", h12, m, ampm);
        }
    }
}

void time_get_date_formatted(char* buf, size_t maxLen) {
    // Current default baseline date
    snprintf(buf, maxLen, "Saturday, Oct 17");
}

} // namespace Services
} // namespace CydOS
