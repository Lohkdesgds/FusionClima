#pragma once

#include <Arduino.h>
#include <esp_adc_cal.h>
#include <driver/adc.h>

#include "data.h"

const uint32_t pin_batt = 37; // 13
const uint32_t pin_vext = 21;
const float batt_smooth_cte = 2.0f;
const float vbatt_precision = 100.0f;

void battery_begin();

void __battery_loop(void*);
