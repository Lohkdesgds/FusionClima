#include "cpuctl.h"

// CPU counters //

struct __i_clk {
    volatile uint32_t m_idl[4]{};
    volatile uint32_t m_tck[4]{};

    __i_clk();
};

__i_clk __cpuclk;
_timed _counter;
const char* TAGCPU = "CPU";

// idle
bool IRAM_ATTR __idl0()
{
    ++__cpuclk.m_idl[0];
    return true;
}
bool IRAM_ATTR __idl1()
{
    ++__cpuclk.m_idl[1];
    return true;
}

// tick
void IRAM_ATTR __tck0()
{
    auto& ref = __cpuclk.m_tck[0]; // working

    if ((ref = ((ref + 1) % max_ticks_per_cpy)) == 0) { // total ticks count
        if (++_counter.low == 0) ++_counter.high;

        __cpuclk.m_tck[2] = std::exchange(ref, 0);
        __cpuclk.m_idl[2] = std::exchange(__cpuclk.m_idl[0], 0);
    }
}
void IRAM_ATTR __tck1()
{
    auto& ref = __cpuclk.m_tck[1]; // working

    if ((ref = ((ref + 1) % max_ticks_per_cpy)) == 0) { // total ticks count
        __cpuclk.m_tck[3] = std::exchange(ref, 0);
        __cpuclk.m_idl[3] = std::exchange(__cpuclk.m_idl[1], 0);
    }
}

void __cpu_startctl(void* unnused)
{
    delay(250);
    ESP_LOGI(TAGCPU, "CPU counters about to be hooked...");
    esp_register_freertos_idle_hook_for_cpu(__idl0, 0);
    esp_register_freertos_idle_hook_for_cpu(__idl1, 1);
    esp_register_freertos_tick_hook_for_cpu(__tck0, 0);
    esp_register_freertos_tick_hook_for_cpu(__tck1, 1);
    ESP_LOGI(TAGCPU, "CPU counters hooked up.");
    vTaskDelete(NULL);
}

__i_clk::__i_clk()
{
    ESP_LOGI(TAGCPU, "Initializing CPU counters (async)...");
    //esp_register_freertos_idle_hook(__idl);
    //esp_register_freertos_tick_hook(__tck);
    xTaskCreate(__cpu_startctl, "cpuctl", 2048, nullptr, __cpu_priority_realtime, nullptr);
}

_timed get_time_copy()
{
    return _counter;
}

float get_cpu_usage(const uint8_t id)
{
    if (id >= amount_of_cpus) return 0.0f;
    return __cpuclk.m_idl[id + amount_of_cpus] >= max_ticks_per_cpy ? 
        0.0f :
        ((max_ticks_per_cpy - __cpuclk.m_idl[id + amount_of_cpus]) * 1.0f / max_ticks_per_cpy);
}

float get_cpu_usage_all()
{
    float avg{};
    for(size_t p = 0; p < amount_of_cpus; ++p) avg += get_cpu_usage(p);
    return avg * 1.0f / amount_of_cpus;
}

size_t get_ram_free()
{
    multi_heap_info_t inf;
    heap_caps_get_info(&inf, MALLOC_CAP_8BIT);
    return inf.total_free_bytes;
}

size_t get_ram_total()
{
    multi_heap_info_t inf;
    heap_caps_get_info(&inf, MALLOC_CAP_8BIT);
    return inf.total_allocated_bytes + inf.total_free_bytes;
}

float get_ram_usage()
{
    multi_heap_info_t inf;
    heap_caps_get_info(&inf, MALLOC_CAP_8BIT);
    const float res = 1.0f * inf.total_allocated_bytes / (1.0f * ((inf.total_allocated_bytes + inf.total_free_bytes) + 1));
    if (res < 0.0f) return 0.0f;
    if (res > 1.0f) return 1.0f;
    return res;
}
