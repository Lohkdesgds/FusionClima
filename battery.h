#pragma once

#include <Arduino.h>
#include <esp_adc_cal.h>
#include <driver/adc.h>

#define VBATT_READ_PIN  37 //13
#define VEXT_ENABLE_PIN 21
#define VBATT_SMOOTH_DEF 10
#define VBATT_PRECISION 100 // 100 -> 2 decimals

#define VBATT_SMOOTH 0.5f
#define VBATT_MIN_CERTAIN 2.75f


struct battinf {
    bool _started = false;
    float last_read = -9999.9f;
    float last_read_smooth = -9999.9f;
};

extern battinf _battinf;

void setup_battery();

// commonly 0 to 100f, can be off that. limit_range forces [0..100]
float read_battery_perc(bool limit_range);

float read_battery_perc_cache(bool limit_range);