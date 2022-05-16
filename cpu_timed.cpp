#include "cpu_timed.h"

void TimedAction::_loop()
{
  while(keep_running) {
    while(m_timed.size() == 0){
      yield();
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    const uint64_t noww = get_time_ms();
    uint64_t next_time = 0;
    {
      std::lock_guard<std::recursive_mutex> l(m_timed_mtx);
      for(auto& i : m_timed)
      {
        if (i->next_task_at < noww || i->next_task_at == 0) {
          i->next_task_at = noww + i->delta;
          if (i->returned) {
            i->returned = false;
            m_mng.post(i->func);
          }
        }
        if (i->next_task_at < next_time || next_time == 0) {
          next_time = i->next_task_at;
        }
      }
    }

    if (noww > next_time) continue;
    uint64_t expect_wait_for = next_time - noww;
    if (expect_wait_for > max_wait_ms) expect_wait_for = max_wait_ms;
    if (expect_wait_for < min_wait_ms) expect_wait_for = min_wait_ms;
    
    std::unique_lock<std::mutex> lk(cond_mut);
    bool first_test = true;
    yield();
    cond.wait_for(lk, std::chrono::milliseconds(expect_wait_for), [&]{return (first_test = !first_test);});
  }
}

TimedAction::TimedAction(TaskManager& tm)
  : m_mng(tm)
{
}

TimedAction::~TimedAction()
{
  if (keep_running) {
    keep_running = false;
    cond.notify_one();
    if (m_thr.joinable()) m_thr.join();
  }
}

void TimedAction::begin()
{
  if (!keep_running) {
    keep_running = true;
    m_thr = std::thread([this]{_loop();});
  }
}

void TimedAction::add(std::function<void(void)> f, uint32_t ms, size_t id)
{
  if (!f || ms == 0) return;
  begin();

  std::shared_ptr<timed_data> td = std::make_shared<timed_data>();
  
  td->func = [ptr = td, f]{
    ptr->returned = false; 
    const uint64_t _tim = get_time_ms();
    f(); 
    ptr->delta_taken = get_time_ms() - _tim;
    ptr->returned = true; 
  };
  td->delta = ms;
  td->custom_id = id;
  
  std::lock_guard<std::recursive_mutex> l(m_timed_mtx);
  m_timed.push_back(td);
  cond.notify_one();
}

// if task is meant to run each 3000 ms and it takes 1000 ms, 0.33f
float TimedAction::get_cpu_time_of(size_t id)
{
  std::lock_guard<std::recursive_mutex> l(m_timed_mtx);
  for(const auto& i : m_timed) {
    if (i->custom_id == id){
      return i->delta_taken * 1.0f / i->delta;
    }
  }
  return 0.0f;
}
