/**
 * @file event_bus.h
 * @brief Decoupled System Event Bus for CYD OS
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

namespace CydOS {
namespace Core {

enum EventType {
    EVENT_NONE = 0,
    EVENT_TICK_1S,
    EVENT_TOUCH_ACTIVITY,
    EVENT_BRIGHTNESS_CHANGED,
    EVENT_VOLUME_CHANGED,
    EVENT_WIFI_STATUS_CHANGED,
    EVENT_SD_STATUS_CHANGED,
    EVENT_BATTERY_CHANGED,
    EVENT_APP_LAUNCH,
    EVENT_APP_CLOSE
};

struct SystemEvent {
    EventType type;
    int32_t   param1;
    int32_t   param2;
    void*     ptrData;
};

typedef void (*EventCallback)(const SystemEvent& event, void* userData);

/**
 * @brief Initialize the system event bus
 */
void event_bus_init();

/**
 * @brief Subscribe a callback to receive events
 * @param type Event type to subscribe to (or EVENT_NONE to receive all events)
 * @param callback Function to call when event fires
 * @param userData Custom pointer passed to callback
 * @return Subscription ID (or -1 if table full)
 */
int8_t event_bus_subscribe(EventType type, EventCallback callback, void* userData = nullptr);

/**
 * @brief Publish an event synchronously to all subscribers
 */
void event_bus_publish(const SystemEvent& event);

/**
 * @brief Helper to publish a simple event with integer param
 */
void event_bus_publish_simple(EventType type, int32_t param1 = 0, int32_t param2 = 0);

} // namespace Core
} // namespace CydOS
