#include "display.h"

void Displayer::_loop()
{
  vTaskDelay(pdMS_TO_TICKS(90));
  
  char buf[64];
  sprintf(buf, "Hello %d!", (test = (++test % 100)));
  
  m_dsp.firstPage();
  std::lock_guard<std::mutex> l(custom_text_m);
  do {
    m_dsp.setFont(u8g2_font_t0_12b_tr);
    m_dsp.drawStr(0,10, buf);
    m_dsp.drawStr(0,20, custom_text[0].c_str());
    m_dsp.drawStr(0,30, custom_text[1].c_str());
    m_dsp.drawStr(0,40, custom_text[2].c_str());
    m_dsp.drawStr(0,50, custom_text[3].c_str());
  } while(m_dsp.nextPage());
}
  
Displayer::Displayer()
  : Async(false), m_dsp(U8G2_R0, DISP_CLK_PIN, DISP_DATA_PIN, DISP_RST_PIN)
{
  m_dsp.begin();
  delayed_launch();
}


void Displayer::set_temp_custom_text(const std::string& str, size_t p)
{
  if (p > 3) return;
  std::lock_guard<std::mutex> l(custom_text_m);
  custom_text[p] = str;
}
