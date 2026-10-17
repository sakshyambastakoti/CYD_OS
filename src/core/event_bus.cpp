/**
 * @file event_bus.cpp
 * @brief System Event Bus implementation
 */

#include "event_bus.h"
#include <Arduino.h>

#define MAX_SUBSCRIBERS 16

namespace CydOS {
namespace Core {

struct Subscriber {
    bool          active;
    EventType     type;
    EventCallback callback;
    void*         userData;
};

static Subscriber s_subscribers[MAX_SUBSCRIBERS];

void event_bus_init() {
    for (size_t i = 0; i < MAX_SUBSCRIBERS; i++) {
        s_subscribers[i].active = false;
        s_subscribers[i].callback = nullptr;
    }
}

int8_t event_bus_subscribe(EventType type, EventCallback callback, void* userData) {
    if (callback == nullptr) return -1;

    for (size_t i = 0; i < MAX_SUBSCRIBERS; i++) {
        if (!s_subscribers[i].active) {
            s_subscribers[i].active   = true;
            s_subscribers[i].type     = type;
            s_subscribers[i].callback = callback;
            s_subscribers[i].userData = userData;
            return (int8_t)i;
        }
    }
    Serial.println("[EVENT] Warning: Subscriber table full!");
    return -1;
}

void event_bus_publish(const SystemEvent& event) {
    for (size_t i = 0; i < MAX_SUBSCRIBERS; i++) {
        if (s_subscribers[i].active && s_subscribers[i].callback != nullptr) {
            if (s_subscribers[i].type == EVENT_NONE || s_subscribers[i].type == event.type) {
                s_subscribers[i].callback(event, s_subscribers[i].userData);
            }
        }
    }
}

void event_bus_publish_simple(EventType type, int32_t param1, int32_t param2) {
    SystemEvent evt;
    evt.type    = type;
    evt.param1  = param1;
    evt.param2  = param2;
    evt.ptrData = nullptr;
    event_bus_publish(evt);
}

} // namespace Core
} // namespace CydOS
