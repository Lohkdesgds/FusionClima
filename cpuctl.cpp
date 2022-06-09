#include "cpuctl.h"

__i_clk __cpuclk;

__i_clk::__i_clk()
{
    mprint("[CPU] Starting CPU counter.");
    //esp_register_freertos_idle_hook(__idl);
    //esp_register_freertos_tick_hook(__tck);
    create_task(__tsk, "async_cpu_counter", __cpu_priority_realtime);
}

// idle
bool __idl()
{
    ++__cpuclk.m_t_idle;
    return true;
}
// tick
void __tck()
{
    if (++__cpuclk.m_t_tick == 1000) {
        __cpuclk.m_idle_cpy = __cpuclk.m_t_idle;//__cpuclk.m_t_idle.exchange(0);
        __cpuclk.m_tick_cpy = __cpuclk.m_t_tick;//__cpuclk.m_t_tick.exchange(0);
        __cpuclk.m_t_idle = __cpuclk.m_t_tick = 0;
    }
}
// task
void __tsk(void*)
{   
    taskYIELD();
    sleep_for(250);
    esp_register_freertos_idle_hook(__idl);
    esp_register_freertos_tick_hook(__tck);
    mprint("[CPU] CPU counter hooked and it is monitoring performance now.\n");
    /*while(1) {
        sleep_for(1000);
        __cpuclk.m_idle_cpy = __cpuclk.m_t_idle.exchange(0);
        __cpuclk.m_tick_cpy = __cpuclk.m_t_tick.exchange(0);
        const float tmp = 1.0f - (1.0f * __cpuclk.m_idle_cpy / (__cpuclk.m_tick_cpy + 1));
        if (tmp > 1.0f) __cpuclk.m_ratio = 1.0f;
        else if (tmp < 0.0f) __cpuclk.m_ratio = 0.0f;
        else __cpuclk.m_ratio = tmp;
    }*/
    exit_task();
}

TaskHandle_t create_task(void(*f)(void*), const char* nam, UBaseType_t priority, size_t stac, void* arg, int coreid)
{
    TaskHandle_t _t = nullptr; 
    while (!_t) {
        if (coreid < 0) xTaskCreate(f, nam, stac, NULL, priority, &_t);
        else xTaskCreatePinnedToCore(f, nam, stac, NULL, priority, &_t, coreid % portNUM_PROCESSORS);
        if (!_t) {
            mprint("Could not create a new thread! (name: %s). Trying again in 100 ms\n", nam);
            sleep_for(100);
        }
    }
    return _t;
}

float get_cpu_usage()
{
    const float val = 1.0f - (1.0f * __cpuclk.m_idle_cpy / (__cpuclk.m_tick_cpy + 1));
    return val > 1.0f ? 1.0f : (val < 0.0f ? 0.0f : val);
}

uint32_t get_cpu_idle_ticks()
{
    return __cpuclk.m_idle_cpy;
}

uint32_t get_cpu_last_ticks()
{
    return __cpuclk.m_tick_cpy;
}

size_t get_ram_free_bytes()
{
    multi_heap_info_t inf;
    heap_caps_get_info(&inf, MALLOC_CAP_8BIT);
    return inf.total_free_bytes;
}

size_t get_ram_total_bytes()
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

unsigned long long get_time_ms()
{
    return std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::milli>>(std::chrono::system_clock::now().time_since_epoch()).count();
}

AutoTiming::AutoTiming(const uint32_t ms)
    : m_now(get_time_ms()), m_last(get_time_ms() + ms)
{
}

AutoTiming::~AutoTiming()
{
    const auto _nw = get_time_ms();
    if (_nw < m_last) sleep_for(m_last - _nw);
}