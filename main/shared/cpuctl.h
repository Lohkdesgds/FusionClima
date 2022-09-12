#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <esp_task_wdt.h>

#include <chrono>
#include <atomic>
#include <limits>
#include <functional>

#include "tools.h"

// CPU tools //

struct _timed {
    uint64_t high = 0;
    uint64_t low  = 0;
};

constexpr UBaseType_t __cpu_priority_realtime = std::numeric_limits<UBaseType_t>::max() - 1;
constexpr size_t max_ticks_per_cpy = 100; // 100 ticks per sec
constexpr size_t amount_of_cpus = 2; // if changed, look _tck* specific ones and __i_clk size

_timed get_time_copy();

float get_cpu_usage(const uint8_t id);
float get_cpu_usage_all();

size_t get_ram_free();
size_t get_ram_total();
float get_ram_usage();