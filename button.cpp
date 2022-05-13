#include "button.h"

void setup_button()
{
  //touchAttachInterrupt(PRG_BTN, NULL, 0);
  touchAttachInterrupt(PRG_BTN, attachBtn, 1); // 1 == least sensitive
  //attachInterrupt(PRG_BTN, attachBtn, CHANGE);
  //esp_sleep_enable_touchpad_wakeup();
}

void prepare_button_deep_sleep()
{
  //detachInterrupt(PRG_BTN);
  touchAttachInterrupt(PRG_BTN, attachBtn, 1);
  esp_sleep_enable_touchpad_wakeup();
}

void on_button_single(void (*func)())
{
  _btn.do_single = func;
}

void on_button_double(void (*func)())
{
  _btn.do_double = func;
}

void attachBtn()
{
  const bool rd = digitalRead(PRG_BTN) == LOW;
  const unsigned long ms = micros() / 1000;

  if (ms - _btn.last_click < btn_debounce) {
    _btn.last_click = ms;
    return;
  }
  
  //if (rd) _btn.do_quick();
  //if (_btn.stat && !rd) { // pressed
    if (ms - _btn.last_click < btn_slower_kind) {
      if (_btn.do_double) _btn.do_double();
    }
    else {
      if (_btn.do_single) _btn.do_single();
    }
  //}
  _btn.last_click = ms;
  _btn.stat = rd;
}
