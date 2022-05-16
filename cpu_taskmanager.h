#pragma once

#include "cpu_tools.h"

#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>

#define TASK_STACK_DEFAULT 4096
constexpr size_t limit_tasks_queue = 20;
constexpr size_t num_threads = 8;
constexpr size_t priority_threads = 1;//tskIDLE_PRIORITY; // 1;

struct event_fire {
  // CPU speed control
  static std::atomic<size_t> task_counter; // tasks at the same time
  static std::mutex task_controller; // locks on cpu clock change

  // TASK control
  std::vector<std::function<void(void)>> m_post;
  std::condition_variable cond;
  std::mutex mut;

  // AUX stuff
  bool keep_running = false;
  bool ack_end[num_threads]{};
};

class TaskManager {
  event_fire m_ev;  
  TaskHandle_t m_thrs[num_threads];
public:
  TaskManager();
  ~TaskManager();

  void post(std::function<void(void)>);

  size_t queue_size() const;
};

void __task_event(void*);
