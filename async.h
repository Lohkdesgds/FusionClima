#pragma once

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <thread>

class Async {
  std::thread m_thr;
  bool m_keep = false;
  void _async();
protected:
  virtual void _loop() = 0;
public:
  Async(bool launch_now);
  ~Async();

  Async(Async&&) = delete;
  Async(const Async&) = delete;
  void operator=(Async&&) = delete;
  void operator=(const Async&) = delete;
  
  void delayed_launch();
  void early_stop();
};
