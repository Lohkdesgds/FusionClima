#pragma once

#include <Arduino.h>
#include <esp_adc_cal.h>
#include <driver/adc.h>

#include "debug_wrapper.h"
#include "cpuctl.h"

#include <mutex>

constexpr int32_t VBATT_READ_PIN    = 37; // 13 on older models
constexpr int32_t VEXT_ENABLE_PIN   = 21;
constexpr int32_t VBATT_SMOOTH_DEF  = 10;
constexpr int32_t VBATT_PRECISION   = 100; // 100 -> 2 decimals

constexpr int32_t VBATT_SMOOTH      = 2.25f;


struct __i_batt {
    float m_level = 0.0f;
    //std::mutex vext_mtx;

    __i_batt();
};

extern __i_batt __batt;

// read once and get percentage [0.0..1.0]. Bool: truncate 0..1?
float get_battery_perc(const bool = true);