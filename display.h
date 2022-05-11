#pragma once

#include <U8g2lib.h>
#include <Arduino.h>
#include <thread>
#include <string>

#ifndef DISP_CLK_PIN
#define DISP_CLK_PIN 15
#endif
#ifndef DISP_DATA_PIN
#define DISP_DATA_PIN 4
#endif
#ifndef DISP_RST_PIN
#define DISP_RST_PIN 16
#endif

//U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, DISP_CLK_PIN, DISP_DATA_PIN, DISP_RST_PIN);

class Display {
  U8G2_SSD1306_128X64_NONAME_1_SW_I2C m_dsp;
  std::thread m_thr;
  bool keep_looping = false;

  void _loop();
public:
  Display();
  ~Display();  
};
