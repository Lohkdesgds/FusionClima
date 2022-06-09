#pragma once

#include <Arduino.h>
#include <mutex>

constexpr int32_t ESP_MODE_PIN = 17;
constexpr int32_t ESP_LED = 25;

struct __i_dbg {
    std::mutex mtx;
    __i_dbg();
};

extern __i_dbg __dbg;

#define mprint(...) { std::lock_guard<std::mutex> l(__dbg.mtx); Serial.printf(__VA_ARGS__); }

void set_led(const bool);
bool get_is_host();