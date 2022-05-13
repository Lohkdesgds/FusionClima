#pragma once

#include <Arduino.h>

// for GPIO0 only (PRG)
constexpr int32_t PRG_BTN = 0; // LOW when pressed
constexpr unsigned long btn_slower_kind = 250; // ms
constexpr unsigned long btn_debounce = 50; // ms

struct button_stat {
  bool stat = true;
  decltype(millis()) last_click = 0;
  void (*do_single)() = nullptr;
  void (*do_double)() = nullptr;
};

static button_stat _btn;

void setup_button();
void prepare_button_deep_sleep();

void on_button_single(void (*func)());
void on_button_double(void (*func)());

void attachBtn();
