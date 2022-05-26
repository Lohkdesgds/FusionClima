#include "cpu_tools.h"

__i_clk_mng __clk_mng;
__i_cpu_info __ecpu[portNUM_PROCESSORS];
TaskHandle_t __thrcpu = nullptr;

TaskHandle_t new_thread(void(*f)(void*), const char* nam, UBaseType_t priority, size_t stac, void* arg, int coreid)
{
    TaskHandle_t _t = nullptr; 
    while (!_t) {
        if (coreid < 0) xTaskCreate(f, nam, stac, NULL, priority, &_t);
        else xTaskCreatePinnedToCore(f, nam, stac, NULL, priority, &_t, coreid % portNUM_PROCESSORS);
        if (!_t) {
            Serial.printf("Could not create a new thread! (name: %s). Trying again in 500 ms\n", nam);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
    return _t;
}

bool __idle_call()
{
    const size_t id = xPortGetCoreID();
    ++__ecpu[id].ticks_idle;
    ++__ecpu[id]._clk_idle;
    return true;
}

void __tick_call()
{
    const size_t id = xPortGetCoreID();
    ++__ecpu[id].ticks_totl;
    ++__ecpu[id]._clk_cnnt;
}

void __calc_clock(void*)
{
    while(1) {
        vTaskDelay(cpu_check_clock_every / portTICK_PERIOD_MS);

        const auto _clk_idl = []{uint32_t v = 0; for(auto& i : __ecpu) v += i._clk_idle.exchange(0); return v; }();
        const auto _clk_totl = []{uint32_t v = 0; for(auto& i : __ecpu) v += i._clk_cnnt.exchange(0); return v; }();
        if (_clk_totl == 0) continue;
        
        __clk_mng._curr_clock_mhz = cpu_clocks_possible[constrain( map(_clk_idl, 0, _clk_totl, - cpu_clocks_possible_len - 1, cpu_clocks_possible_len), 0, cpu_clocks_possible_len - 1)];

        setCpuFrequencyMhz(__clk_mng._curr_clock_mhz);
        yield();
    }
}

void __calc_loop(void*)
{    
    while (1) {
        for(auto& ecp : __ecpu) {
            const auto _idle = ecp.ticks_idle.exchange(0);
            const auto _totl = ecp.ticks_totl.exchange(0);
            
            const float tmp = 1.0f - (1.0f * _idle / (_totl + 1));
            
            if (tmp > 1.0f) ecp.last_perc = 1.0f;
            else if (tmp < 0.0f) ecp.last_perc = 0.0f;
            else ecp.last_perc = tmp;
            
            vTaskDelay(333 / portTICK_PERIOD_MS);
        }
        yield();
    }
    vTaskDelete(NULL);
}

void setup_cpu_tools(bool autoclock)
{
    if (__thrcpu) return;
    __clk_mng._thrautoclock = autoclock;
    
    for(size_t i = 0; i < portNUM_PROCESSORS; ++i) {
        if (esp_register_freertos_idle_hook_for_cpu(__idle_call, i) != ESP_OK) {
            Serial.printf("Could not hook idle correcly for CPU#%i\n", i);
            while(1);
        }
        if (esp_register_freertos_tick_hook_for_cpu(__tick_call, i) != ESP_OK) {
            Serial.printf("Could not hook tick correcly for CPU#%i\n", i);
            while(1);
        }
    }
    
    __thrcpu = new_thread(__calc_loop, "perfmon", priority_real_time, 3072);
    if (__clk_mng._thrautoclock) __clk_mng._thr_auto = new_thread(__calc_clock, "perfctl", priority_real_time, 2048);
}

void stop_cpu_tools()
{
    if (__thrcpu) vTaskDelete(__thrcpu);
    if (__clk_mng._thr_auto) vTaskDelete(__clk_mng._thr_auto);
    __thrcpu = nullptr;
    __clk_mng._thr_auto = nullptr;
}

uint32_t get_cpu_clock()
{
    return __clk_mng._thrautoclock ? __clk_mng._curr_clock_mhz : getCpuFrequencyMhz();
}

uint16_t get_cpu_count(){
    return portNUM_PROCESSORS;
}

float get_cpu_usage()
{
    if (!__thrcpu) setup_cpu_tools(true);
    
    float _tot = 0.0f;
    for(size_t i = 0; i < portNUM_PROCESSORS; ++i) {
        _tot += __ecpu[i].last_perc;
    }
    const float res = _tot * 1.0f / portNUM_PROCESSORS;
    if (res < 0.0f) return 0.0f;
    if (res > 1.0f) return 1.0f;
    return res;
}

float get_cpu_usage_id(size_t p)
{
    if (!__thrcpu) setup_cpu_tools(true);
    
    if (p >= portNUM_PROCESSORS) return 0.0f;
    const float res = __ecpu[p].last_perc;
    if (res < 0.0f) return 0.0f;
    if (res > 1.0f) return 1.0f;
    return res;
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

void set_auto_clock(bool d)
{
    __clk_mng._thrautoclock = d;
    if (!d) {
        if (__clk_mng._thr_auto) vTaskDelete(__clk_mng._thr_auto);
        __clk_mng._thr_auto = nullptr;
        setCpuFrequencyMhz(cpu_clocks_possible[0]);
    }
    else {
        if (!__clk_mng._thr_auto) __clk_mng._thr_auto = new_thread(__calc_clock, "perfctl", priority_real_time, 2048);
    }
}

unsigned long long get_time_ms()
{
        return std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::milli>>(std::chrono::system_clock::now().time_since_epoch()).count();
}
