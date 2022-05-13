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

enum class e_display_mode {RECEIVER_TEMP,RECEIVER_HUM,RECEIVER_HEATINDEX,RECEIVER_LASTTIME,SENDER_DEFAULT};

constexpr decltype(millis()) max_timeout = 15000; // msec

// async
class Displayer : public Async {
  U8G2_SSD1306_128X64_NONAME_F_SW_I2C m_dsp;

  e_display_mode m_mode = e_display_mode::RECEIVER_TEMP;

  // RECEIVER
  int m_rssi = 20; // dB
  float m_snr = 0.0f; // dB
  bool m_once_set = false;

  // SENDER
  decltype(millis()) sending_at = 0;
  int m_gain = 2; // dB

  // COMMON
  decltype(millis()) m_last_upd = 0;
  float m_temp = 25.6f; // celsius
  float m_hum = 69.0f; // perc
  bool sleepin = false;

  void mode_send_default();
  void mode_recv_default();
  
  void _loop();
public:
  Displayer(); 

  void set_mode(e_display_mode);

  // based on RECV/SEND mode
  void next_mode();

  void receiver(int rssi, float snr, float temp, float hum);
  void sender(unsigned long time_send, float temp, float hum, int gain);

  void sleeping(bool);
};
