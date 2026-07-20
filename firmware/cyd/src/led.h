#pragma once
#include <Arduino.h>
#include "config.h"

#define LED_STATUS_TIMEOUT_MS 30000UL

static uint32_t _led_status_ts = 0;  // non-zero = status is active, will auto-idle

inline void led_set(bool r, bool g, bool b) {
    // RGB LED is active LOW: true = on, false = off
    digitalWrite(LED_R, r ? LOW : HIGH);
    digitalWrite(LED_G, g ? LOW : HIGH);
    digitalWrite(LED_B, b ? LOW : HIGH);
}

inline void led_idle() {
    _led_status_ts = 0;
    led_set(0, 0, 0);
}

inline void led_generating() {
    _led_status_ts = 0;   // generating has no timeout
    led_set(0, 0, 1);     // blue
}

inline void led_ok() {
    _led_status_ts = millis();
    led_set(0, 1, 0);     // green
}

inline void led_error() {
    _led_status_ts = millis();
    led_set(0, 1, 1);     // cyan (G+B) — LED_R/GPIO4 not wired on CYD
}

// call from loop() — resets to idle 30 s after ok/error
inline void led_tick() {
    if (_led_status_ts && millis() - _led_status_ts >= LED_STATUS_TIMEOUT_MS) {
        led_idle();
    }
}
