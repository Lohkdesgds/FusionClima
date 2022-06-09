#pragma once

#include "debug_wrapper.h"

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "esp_heap_caps.h"
#include <esp_task_wdt.h>

#include <chrono>
#include <atomic>
#include <limits>
#include <functional>

// CPU counters //

struct __i_clk {
    volatile uint32_t m_t_idle;
    volatile uint32_t m_t_tick;
    uint32_t m_idle_cpy = 0, m_tick_cpy = 0;
    float m_ratio = 0.0f; // [0.0..1.0]

    __i_clk();
};

extern __i_clk __cpuclk;

// idle
bool __idl();
// tick
void __tck();
// task
void __tsk(void*);

// CPU tools //

constexpr UBaseType_t __cpu_priority_realtime = std::numeric_limits<UBaseType_t>::max() - 1;
constexpr size_t __cpu_stack_default = 16384;

TaskHandle_t create_task(void(*f)(void*), const char* nam, UBaseType_t priority = tskIDLE_PRIORITY, size_t stac = __cpu_stack_default, void* arg = nullptr, int coreid = -1);

// auto create task auto
#define acta(FUNCNAME) { create_task(FUNCNAME, #FUNCNAME); }
// auto create task priority
#define actp(FUNCNAME, PRIORITY) { create_task(FUNCNAME, #FUNCNAME, PRIORITY); }
// auto create task core
#define actc(FUNCNAME, COREID) { create_task(FUNCNAME, #FUNCNAME, tskIDLE_PRIORITY, __cpu_stack_default, nullptr, COREID); }
// auto create task core pinned
#define actcp(FUNCNAME, COREID, PRIORITY) { create_task(FUNCNAME, #FUNCNAME, PRIORITY, __cpu_stack_default, nullptr, COREID); }

#define exit_task() vTaskDelete(NULL)
#define sleep_for(MILLISEC) {taskYIELD(); vTaskDelay(MILLISEC / portTICK_PERIOD_MS); }

float get_cpu_usage();
uint32_t get_cpu_idle_ticks();
uint32_t get_cpu_last_ticks();
size_t get_ram_free_bytes();
size_t get_ram_total_bytes();
float get_ram_usage();
unsigned long long get_time_ms();

class AutoTiming {
    const unsigned long long m_now, m_last;
public:
    AutoTiming(const uint32_t ms);
    ~AutoTiming();
};