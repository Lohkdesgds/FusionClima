#pragma once

#include <U8g2lib.h>
#include <Arduino.h>
#include "sensor.h"
#include "async.h"
#include <string>
#include <mutex>
#include <chrono>

#ifndef DISP_CLK_PIN
#define DISP_CLK_PIN 15
#endif
#ifndef DISP_DATA_PIN
#define DISP_DATA_PIN 4
#endif
#ifndef DISP_RST_PIN
#define DISP_RST_PIN 16
#endif

#include "shared.h"
#include "display_bitmaps.h"
#include "cpu_tools.h"

enum class e_display_mode {RECEIVER_TEMP,RECEIVER_HUM,RECEIVER_HEATINDEX,RECEIVER_LASTTIME,SENDER_DEFAULT};

struct display_data {
  unsigned long long last_received = 0; // expects get_time_ms()
  float cpu_usage_perc = 0.0f; // expect [0.0..1.0]
  float ram_free_perc = 0.0f; // expect [1.0..0.0]
  uint16_t signals_received = 0; // expect [0..99]
  uint16_t signal_rssi = 0; // expects [0..100], inverse of rssi
  float batt_perc = 0.0f; // expects [0.0..1.0]
  bool debug_literal = false; // shows raw text instead of icons
};

class Displayer {
  U8G2_SSD1306_128X64_NONAME_F_SW_I2C* m_dsp = nullptr;
  
  display_data m_data;

  // if not debug_literal:
  void draw_top();
  void draw_bottom();

  // if debug_literal:
  void draw_debug();

  void flip();
public:
  ~Displayer();
  
  display_data& get();
  
  void draw();
  void destroy();
};
