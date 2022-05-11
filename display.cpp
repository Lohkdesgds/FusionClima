#include "display.h"

void Display::_loop()
{
  int test = 0;
  m_dsp.begin();

  char buf[64];
  
  while(keep_looping){
    sprintf(buf, "Hello %d!", (test = (++test % 20)));
    
    m_dsp.firstPage();
    do {
      m_dsp.setFont(u8g2_font_ncenB14_tr);
      m_dsp.drawStr(0,24, buf);
    } while(m_dsp.nextPage());
    
    delay(1);
  }
}
  
Display::Display()
  : m_dsp(U8G2_R0, DISP_CLK_PIN, DISP_DATA_PIN, DISP_RST_PIN)
{
  keep_looping = true;
  m_thr = std::thread([this]{ _loop(); });
}

Display::~Display()
{
  keep_looping = false;
  if (m_thr.joinable()) m_thr.join();
}
