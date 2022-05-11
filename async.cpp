#include "async.h"

void Async::_async()
{
  while(m_keep) {
    _loop();
    vTaskDelay(pdMS_TO_TICKS(10)); // watchdog easy fix
    esp_task_wdt_reset(); // guaranteed watchdog reset
  }
}

Async::Async(bool launch_now)
{
  if (launch_now) delayed_launch();
}

Async::~Async()
{
  early_stop();
}

void Async::delayed_launch()
{
  if (m_keep) return;  
  m_keep = true;
  m_thr = std::thread([this]{_async();});
}

void Async::early_stop()
{
  if (!m_keep) return;
  m_keep = false;
  if (m_thr.joinable()) m_thr.join();
}
