#include "cpu_taskmanager.h"

TaskManager::task_manager Tasker;
TaskTimed::task_timed TimedTasker(Tasker);

namespace TaskManager {
    
    std::atomic<size_t> event_fire::task_counter;
    std::mutex event_fire::task_controller;

    task_manager::task_manager()
    {
        m_ev.keep_running = true;
        for(size_t p = 0; p < num_threads; ++p) {
            char nam[32];
            sprintf(nam, "tskmng%x", p);
            xTaskCreatePinnedToCore(__task_event, nam, TASK_STACK_DEFAULT, (void*)&m_ev, priority_threads, &m_thrs[p], p % portNUM_PROCESSORS);    
        }  
    }

    task_manager::~task_manager()
    {
        m_ev.keep_running = false;
        for(const auto& i : m_ev.ack_end) {
            while(!i) std::chrono::milliseconds(50);
        }
    }

    void task_manager::post(std::function<void(void)> f)
    {
        if (!f) return;
        setup_cpu_tools(true);
        while (m_ev.m_post.size() >= limit_tasks_queue) vTaskDelay(pdMS_TO_TICKS(250));
        
        std::lock_guard<std::mutex> l(m_ev.mut);
        m_ev.m_post.push_back(f);
        m_ev.cond.notify_one();
    }

    size_t task_manager::queue_size() const
    {
        return m_ev.m_post.size();
    }
      
    void __task_event(void* arr){
        esp_task_wdt_init(30, false);
        event_fire& ev = *(event_fire*)arr;

        ev.ack_end[xPortGetCoreID()] = false;

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
}

namespace TaskTimed {
    
    void task_timed::_loop()
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
                        if (i->next_task_at + i->delta < noww) i->next_task_at = noww + i->delta + (i->randomness != 0 ? (random(0, i->randomness)) : 0);
                        else i->next_task_at += (i->delta + (i->randomness != 0 ? (random(0, i->randomness)) : 0));
                        
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

    task_timed::task_timed(TaskManager::task_manager& tm)
        : m_mng(tm)
    {
    }

    task_timed::~task_timed()
    {
        if (keep_running) {
            keep_running = false;
            cond.notify_one();
            if (m_thr.joinable()) m_thr.join();
        }
    }

    void task_timed::begin()
    {
        if (!keep_running) {
            keep_running = true;
            m_thr = std::thread([this]{_loop();});
        }
    }

    void task_timed::add(std::function<void(void)> f, uint32_t ms, size_t id, uint32_t randomness)
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
        td->randomness = randomness;
        td->custom_id = id;
        
        std::lock_guard<std::recursive_mutex> l(m_timed_mtx);
        m_timed.push_back(td);
        cond.notify_one();
    }

    // if task is meant to run each 3000 ms and it takes 1000 ms, 0.33f
    float task_timed::get_cpu_time_of(size_t id)
    {
        std::lock_guard<std::recursive_mutex> l(m_timed_mtx);
        for(const auto& i : m_timed) {
            if (i->custom_id == id){
                return i->delta_taken * 1.0f / i->delta;
            }
        }
        return 0.0f;
    }

}