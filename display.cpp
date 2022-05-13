#include "display.h"

void Displayer::mode_send_default()
{
  static char sprintbuf[64];
  
  m_dsp.setFont(u8g2_font_t0_11_te);
  
  sprintf(sprintbuf, u8"Sender mode | %i dB", m_gain);    
  m_dsp.drawUTF8(0, 8, sprintbuf);
  
  sprintf(sprintbuf, u8"Temp: %.2f ºC", m_temp);
  m_dsp.drawUTF8(0, 17, sprintbuf);

  sprintf(sprintbuf, u8"Humidity: %.2f%%", m_hum);
  m_dsp.drawUTF8(0, 26, sprintbuf); 

  if (sending_at > millis()) sprintf(sprintbuf, u8"Sending in %.1f...", ((sending_at - millis()) * 1.0f / 1000.0f));
  else sprintf(sprintbuf, u8"Sended.");
  m_dsp.drawUTF8(0, 35, sprintbuf);  
  
  sprintf(sprintbuf, u8"UT50d:");
  m_dsp.drawUTF8(0, 44, sprintbuf);
  
  sprintf(sprintbuf, u8"*%lus", (millis() / 1000));
  m_dsp.drawUTF8(0, 53, sprintbuf);
  
  sprintf(sprintbuf, u8"%s", VERSION.c_str());
  m_dsp.drawUTF8(0, 62, sprintbuf);
}

void Displayer::mode_recv_default()
{
  // bars
  const bool timedout = (millis() > (m_last_upd + max_timeout)) || (millis() < (m_last_upd - max_timeout));
  static char sprintbuf[64];

  if (!timedout) {
    m_dsp.drawBox(114, 6, 2, 2);
    if (m_rssi >= -100.0f) m_dsp.drawBox(117, 5, 2, 3);
    else                   m_dsp.drawBox(117, 7, 2, 1);
    if (m_rssi >= -85.0f)  m_dsp.drawBox(120, 3, 2, 5);
    else                   m_dsp.drawBox(120, 7, 2, 1);
    if (m_rssi >= -65.0f)  m_dsp.drawBox(123, 1, 2, 7);
    else                   m_dsp.drawBox(123, 7, 2, 1);
    if (m_rssi >= -40.0f)  m_dsp.drawBox(126, 0, 2, 8);
    else                   m_dsp.drawBox(126, 7, 2, 1);
    m_dsp.drawXBM(104, 0, 8, 8, u8g_ok); // OK sign
  }
  else {
    m_dsp.drawBox(114, 7, 2, 1);
    m_dsp.drawBox(117, 7, 2, 1);
    m_dsp.drawBox(120, 7, 2, 1);
    m_dsp.drawBox(123, 7, 2, 1);
    m_dsp.drawBox(126, 7, 2, 1);
    m_dsp.drawXBM(104, 0, 8, 8, u8g_fail); // FAIL sign    
  }

  if (!timedout) {
    float prop = (millis() - m_last_upd) * 1.0f / max_timeout;
    if (prop < 0.0f) prop = 0.0f;
    if (prop > 1.0f) prop = 1.0f;
    int first = (int)(128 * prop) - 3;
    if (first < 0) first = 0;
    
    m_dsp.drawLine(0, 13, first, 13);
    m_dsp.drawLine((int)(128 * prop), 13, 128, 13);
  }
  else {
    m_dsp.drawLine(0, 12, 128, 12);
    m_dsp.drawLine(0, 14, 128, 14);
  }
  

  m_dsp.setFont(u8g2_font_t0_11_te);

  if (m_once_set) {
    switch ((millis() / 1500) % 5){
    case 0:
      sprintf(sprintbuf, u8"SNR: %.1f dB", m_snr);
      break;
    case 1:
      sprintf(sprintbuf, u8"GAIN: %i dB", m_rssi);
      break;
    case 2:
      sprintf(sprintbuf, u8"TEMP: %.2f ºC", m_temp);
      break;
    case 3:
      sprintf(sprintbuf, u8"%s", VERSION.c_str());
      break;
    default:
      sprintf(sprintbuf, u8"HUM: %.2f%%", m_hum);
      break;
    }  
  }
  else {
      sprintf(sprintbuf, u8"Waiting...");
  }
  m_dsp.drawUTF8(0, 8, sprintbuf);

  switch(m_mode) {
  case e_display_mode::RECEIVER_TEMP:
    sprintf(sprintbuf, u8"TEMPERATURE");
    break;
  case e_display_mode::RECEIVER_HUM:
    sprintf(sprintbuf, u8"HUMIDITY");
    break;
  case e_display_mode::RECEIVER_HEATINDEX:
    sprintf(sprintbuf, u8"HEAT INDEX");
    break;
  case e_display_mode::RECEIVER_LASTTIME:
    sprintf(sprintbuf, u8"LAST TIME");
    break;
  }

  {
    auto wid = m_dsp.getUTF8Width(sprintbuf);
    m_dsp.drawUTF8((128 - wid) / 2, 29, sprintbuf);    
    m_dsp.drawLine((128 - wid) / 2, 31, 128 - ((128 - wid) / 2), 31);
  }

  if (m_once_set) {
    switch(m_mode) {
    case e_display_mode::RECEIVER_TEMP:
      if (!timedout) sprintf(sprintbuf, u8"%.2f ºC", m_temp);
      else           sprintf(sprintbuf, u8"%.2f ºC*", m_temp);
      break;
    case e_display_mode::RECEIVER_HUM:
      if (!timedout) sprintf(sprintbuf, u8"%.3f%%", m_hum);
      else           sprintf(sprintbuf, u8"%.3f%%*", m_hum);
      break;
    case e_display_mode::RECEIVER_HEATINDEX:
      {
        if (!timedout) sprintf(sprintbuf, u8"%.2f ºC", computeHeatIndex(m_temp, m_hum, false));
        else           sprintf(sprintbuf, u8"%.2f ºC*", computeHeatIndex(m_temp, m_hum, false));
      }
      break;
    case e_display_mode::RECEIVER_LASTTIME:
      {
        const auto tt = millis();
        if (tt - m_last_upd > 60000) {
          sprintf(sprintbuf, u8"> 60 sec");
        }
        else {
          sprintf(sprintbuf, u8"%.1f sec", ((tt - m_last_upd) * 1.0f / 1000.0f));
        }
      }
      break;
    }
  }
  else {
    sprintf(sprintbuf, u8"...");
  }

  {
    m_dsp.setFont(u8g2_font_osb21_tf);
    auto wid = m_dsp.getUTF8Width(sprintbuf);  
    m_dsp.drawUTF8((128 - wid) / 2, 59, sprintbuf);
  }
}

void Displayer::_loop()
{
  vTaskDelay(pdMS_TO_TICKS(90));

  if (sleepin) {
    m_dsp.clearDisplay();
    vTaskDelay(pdMS_TO_TICKS(490));
    return;
  }
  
  m_dsp.clearBuffer();
  //m_dsp.firstPage();
  //do {
  switch(m_mode) {
  case e_display_mode::RECEIVER_TEMP:
  case e_display_mode::RECEIVER_HUM:
  case e_display_mode::RECEIVER_LASTTIME:
  case e_display_mode::RECEIVER_HEATINDEX:
    mode_recv_default();
    break;
  case e_display_mode::SENDER_DEFAULT:
    mode_send_default();
    break;
  }
  //} while(m_dsp.nextPage());
  m_dsp.sendBuffer();
}

Displayer::Displayer()
  : Async(false), m_dsp(U8G2_R0, DISP_CLK_PIN, DISP_DATA_PIN, DISP_RST_PIN)
{
  m_dsp.begin();
  m_dsp.enableUTF8Print();
  delayed_launch();
}

void Displayer::set_mode(e_display_mode mo)
{
  m_mode = mo;
}

void Displayer::next_mode()
{
  switch(m_mode) {
  case e_display_mode::RECEIVER_TEMP:
    m_mode = e_display_mode::RECEIVER_HUM;
    break;
  case e_display_mode::RECEIVER_HUM:
    m_mode = e_display_mode::RECEIVER_HEATINDEX;
    break;
  case e_display_mode::RECEIVER_HEATINDEX:
    m_mode = e_display_mode::RECEIVER_LASTTIME;
    break;
  case e_display_mode::RECEIVER_LASTTIME:
    m_mode = e_display_mode::RECEIVER_TEMP;
    break;
  case e_display_mode::SENDER_DEFAULT:
    m_mode = e_display_mode::SENDER_DEFAULT;
    break;
  }
}

void Displayer::receiver(int rssi, float snr, float temp, float hum)
{
  m_rssi = rssi;
  m_snr = snr;
  m_temp = temp;
  m_hum = hum;
  m_last_upd = millis();
  m_once_set = true;
}

void Displayer::sender(unsigned long time_send, float temp, float hum, int gain)
{
  m_temp = temp;
  m_hum = hum;
  m_gain = gain;
  m_last_upd = millis();
  sending_at = millis() + time_send;
}

void Displayer::sleeping(bool sleeps)
{
  sleepin = sleeps;
}

/*void Displayer::_loop()
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
}*/
