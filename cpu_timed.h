#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include <esp_task_wdt.h>

#include "cpu_tools.h"
#include "cpu_taskmanager.h"

#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <memory>

constexpr uint64_t max_wait_ms = 300, min_wait_ms = 50;

struct timed_data {
  std::function<void(void)> func;
  uint64_t next_task_at = 0;
  bool returned = true;

  // CONFIG
  uint32_t delta = 0, delta_taken = 0; // ms
  size_t custom_id = 0;
};

class TimedAction {
  std::vector<std::shared_ptr<timed_data>> m_timed;
  std::recursive_mutex m_timed_mtx;
  
  TaskManager& m_mng;
  std::thread m_thr; // easier for this case.
  
  std::condition_variable cond;
  std::mutex cond_mut;
  
  bool keep_running;

  void _loop();
public:
  TimedAction(TaskManager&);
  ~TimedAction();

  void begin();
  void add(std::function<void(void)> f, uint32_t ms, size_t id);

  // if task is meant to run each 3000 ms and it takes 1000 ms, 0.33f
  float get_cpu_time_of(size_t id);
};
