#pragma once

#include <Arduino.h>
#include <esp_adc_cal.h>
#include <driver/adc.h>

#define VBATT_READ_PIN  13
#define VEXT_ENABLE_PIN 21
#define VBATT_SMOOTH_DEF 16
#define VBATT_PRECISION 100 // 100 -> 2 decimals
//#define COEF_F_DISCHARGE_TEST 0.75f // 7.5 decimals
#define VBATT_AVG_ADPT 0.5f // pt for old val
#define VBATT_LIMIT_TENDENCY 5

struct battinf {
  float last_read = -9999.9f;
  int tendency = -3; // max -20/20, if ++, goes up, else goes down.
};

static battinf _battinf;

void setup_battery();

// commonly 0 to 100f, can be off that. limit_range forces [0..100]
float read_battery_perc(bool limit_range);

float read_battery_perc_cache(bool limit_range);

// must keep read_battery_perc up to date.
bool get_battery_charging();
