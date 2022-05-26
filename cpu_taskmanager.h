#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include <esp_task_wdt.h>

#include "cpu_tools.h"

#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <chrono>
#include <thread>

#define TASK_STACK_DEFAULT 4096

namespace TaskManager {
    
    constexpr size_t limit_tasks_queue = 20;
    constexpr size_t num_threads = 2;
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

    class task_manager {
        event_fire m_ev;  
        TaskHandle_t m_thrs[num_threads];
    public:
        task_manager();
        ~task_manager();
        
        void post(std::function<void(void)>);
        
        size_t queue_size() const;
    };

    void __task_event(void*);    
}

namespace TaskTimed {
    
    constexpr uint64_t max_wait_ms = 500, min_wait_ms = 50;

    struct timed_data {
        std::function<void(void)> func;
        uint64_t next_task_at = 0;
        bool returned = true;
        
        // CONFIG
        uint32_t delta = 0, randomness = 0, delta_taken = 0; // ms
        size_t custom_id = 0;
    };

    class task_timed {
        std::vector<std::shared_ptr<timed_data>> m_timed;
        std::recursive_mutex m_timed_mtx;
        
        TaskManager::task_manager& m_mng;
        std::thread m_thr; // easier for this case.
        
        std::condition_variable cond;
        std::mutex cond_mut;
        
        bool keep_running;
        
        void _loop();
    public:
        task_timed(TaskManager::task_manager&);
        ~task_timed();
        
        void begin();
        void add(std::function<void(void)> f, uint32_t ms, size_t id, uint32_t randomness = 0);
        
        // if task is meant to run each 3000 ms and it takes 1000 ms, 0.33f
        float get_cpu_time_of(size_t id);
    };
}

extern TaskManager::task_manager Tasker;
extern TaskTimed::task_timed TimedTasker;