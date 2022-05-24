#include "cpu_taskmanager.h"

std::atomic<size_t> event_fire::task_counter;
std::mutex event_fire::task_controller;

TaskManager::TaskManager()
{
  m_ev.keep_running = true;
  for(size_t p = 0; p < num_threads; ++p) {
    xTaskCreatePinnedToCore(__task_event, "tskmng", TASK_STACK_DEFAULT, (void*)&m_ev, priority_threads, &m_thrs[p], p % portNUM_PROCESSORS);    
  }  
}

TaskManager::~TaskManager()
{
  m_ev.keep_running = false;
  for(const auto& i : m_ev.ack_end) {
    while(!i) std::chrono::milliseconds(50);
  }
}

void TaskManager::post(std::function<void(void)> f)
{
  if (!f) return;
  setup_cpu_tools(true);
  while (m_ev.m_post.size() >= limit_tasks_queue) vTaskDelay(pdMS_TO_TICKS(250));
  
  /*if (min_clock <= 10) min_clock = 10;
  else if (min_clock <= 20) min_clock = 20;
  else if (min_clock <= 40) min_clock = 40;
  else if (min_clock <= 80) min_clock = 80;
  else if (min_clock <= 160) min_clock = 160;
  else min_clock = 240;*/

  //fp.min_clock = min_clock;
  std::lock_guard<std::mutex> l(m_ev.mut);
  m_ev.m_post.push_back(f);
  m_ev.cond.notify_one();
}

size_t TaskManager::queue_size() const
{
  return m_ev.m_post.size();
}
  
void __task_event(void* arr){
  esp_task_wdt_init(30, false);
  event_fire& ev = *(event_fire*)arr;

  ev.ack_end[xPortGetCoreID()] = false;
  
  /*if (ev.task_counter == (unsigned int)0) {
    std::lock_guard<std::mutex> l(ev.task_controller);
    setCpuFrequencyMhz(10);
  }*/
  
  while(ev.keep_running) {
    yield();
    std::unique_lock<std::mutex> lk(ev.mut);
    ev.cond.wait_for(lk, std::chrono::milliseconds(250), [&]{return ev.m_post.size() > 0 || !ev.keep_running;});
    if (!ev.keep_running || ev.m_post.empty()) continue;
    
    const auto func = std::move(ev.m_post.front());
    ev.m_post.erase(ev.m_post.begin());
    if (ev.m_post.size()) ev.cond.notify_one();
    lk.unlock();

    if (func){
      ++ev.task_counter;
      func();
      --ev.task_counter;
    }
    if (ev.m_post.size()) ev.cond.notify_one();
  }
  ev.ack_end[xPortGetCoreID()] = true;
  vTaskDelete(NULL);
}
