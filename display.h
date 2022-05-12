#pragma once

#include <U8g2lib.h>
#include <Arduino.h>
#include "async.h"
#include <string>
#include <mutex>

#ifndef DISP_CLK_PIN
#define DISP_CLK_PIN 15
#endif
#ifndef DISP_DATA_PIN
#define DISP_DATA_PIN 4
#endif
#ifndef DISP_RST_PIN
#define DISP_RST_PIN 16
#endif

// async
class Displayer : public Async {
  U8G2_SSD1306_128X64_NONAME_1_SW_I2C m_dsp;

  std::string custom_text[4];
  std::mutex custom_text_m;

  int test = 0;
  
  void _loop();
public:
  Displayer(); 

  void set_temp_custom_text(const std::string&, size_t);
};
