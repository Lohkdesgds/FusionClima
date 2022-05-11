#include "display.h"

void Display::_loop()
{
  vTaskDelay(pdMS_TO_TICKS(190));
  
  char buf[64];
  sprintf(buf, "Hello %d!", (test = (++test % 20)));
  
  m_dsp.firstPage();
  std::lock_guard<std::mutex> l(custom_text_m);
  do {
    m_dsp.setFont(u8g2_font_t0_12b_tr);
    m_dsp.drawStr(0,24, buf);
    m_dsp.drawStr(0,48, custom_text.c_str());
  } while(m_dsp.nextPage());
}
  
Display::Display()
  : Async(false), m_dsp(U8G2_R0, DISP_CLK_PIN, DISP_DATA_PIN, DISP_RST_PIN)
{
  m_dsp.begin();
  delayed_launch();
}


void Display::set_temp_custom_text(const std::string& str)
{
  std::lock_guard<std::mutex> l(custom_text_m);
  custom_text = str;
}
