#include <Arduino.h>
#include <heltec.h>

#include "benchmark.h"
#include "cpuctl.h"
#include "easybattery.h"
#include "displayer.h"

#include "as_host.h"
#include "as_client.h"

constexpr uint16_t system_key = 0x69;

/*
Referências:
- benchmark.h: do_benchmark() = estresse de CPU.
- cpuctl.h: act* = cria threads
- easybattery.h: get_battery_perc = pega e suaviza direto no comando a porcentagem da bateria.

*/

//std::atomic<int> led_stat;
//
//void draw_loop(void*)
//{
//    while(1) {
//        AutoTiming at(1000);
//        display.draw();
//    }
//}
//
//void test(void*)
//{
//    mprint("Starting test loop...\n");
//    while(1) {
//        set_led(++led_stat > 0);
//        sleep_for(6000);
//        set_led(--led_stat > 0);
//        do_benchmark();
//    }
//}
//
//void monitor(void*)
//{
//    mprint("Starting monitor...\n");
//    while(1)
//    {
//        mprint("Resource usage: %.2f [%u of %u were idle] | BATT: %04.2f\n", get_cpu_usage(), get_cpu_idle_ticks(), get_cpu_last_ticks(), 100.0f * get_battery_perc());
//        sleep_for(1000);
//    }
//}

void setup()
{
    //mprint("Starting stuff in 2 seconds...\n");
    //sleep_for(2000);

    LR.begin(system_key);

    if (get_is_host()) {
        mprint("Waking up as host...\n");
        create_task([](void*){as_host();}, "main", 10, 16384, nullptr, 0);
    }
    else {
        mprint("Waking up as client...\n");
        button_begin(0);
        create_task([](void*){as_client();}, "main", 10, 16384, nullptr, 0);
    }

    //LR.begin(system_key);
    //actc(test, 0);
    //actc(test, 1);
    //actc(monitor, 0);
    //acta(draw_loop);
}

void loop() { vTaskDelete(NULL); } // remove loop entirely