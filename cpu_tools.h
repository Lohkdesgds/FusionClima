#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "esp_heap_caps.h"
#include <esp_task_wdt.h>

#include <limits>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <mutex>

#undef max
#undef min

struct __i_cpu_info {
  std::atomic<uint32_t> ticks_idle;
  std::atomic<uint32_t> ticks_totl;
  std::atomic<uint32_t> _clk_idle;
  std::atomic<uint32_t> _clk_cnnt;
  float last_perc = 0.0f; // [0.0f .. 1.0f]
};

struct __i_clk_mng {
  std::atomic<uint32_t> _clk_count;
  uint32_t _curr_clock_mhz = 240;
  bool _thrautoclock = true;
  TaskHandle_t _thr_auto = nullptr;
};

extern __i_clk_mng __clk_mng;

extern __i_cpu_info __ecpu[portNUM_PROCESSORS];
extern TaskHandle_t __thrcpu;

constexpr uint32_t cpu_clocks_possible_len = 6;
constexpr uint32_t cpu_clocks_possible[cpu_clocks_possible_len] = {240, 160, 80, 40, 20, 10};
constexpr uint32_t cpu_check_clock_every = 250; // ms

constexpr UBaseType_t priority_real_time = std::numeric_limits<UBaseType_t>::max() - 1;

TaskHandle_t new_thread(void(*f)(void*), const char* nam, UBaseType_t priority, size_t stac = 8192, void* arg = nullptr, int coreid = -1);

bool __idle_call();
void __tick_call();
void __calc_clock(void*);
void __calc_loop(void*);

void setup_cpu_tools(bool autoclock);
void stop_cpu_tools();

uint32_t get_cpu_clock();
uint16_t get_cpu_count();
float get_cpu_usage();
float get_cpu_usage_id(size_t);

size_t get_ram_free_bytes();
size_t get_ram_total_bytes();

float get_ram_usage();

void set_auto_clock(bool);

unsigned long long get_time_ms();
