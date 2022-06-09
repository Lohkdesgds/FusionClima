#include <heltec.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_freertos_hooks.h>
#include <esp_task_wdt.h>

#include "data.h"
#include "calc.h"
#include "button.h"
#include "dhtmeasure.h"
#include "mylora.h"
#include "displaying.h"
#include "battery.h"

//void task_measure(void*);

TaskHandle_t thr_measurer;

const uint32_t pin_dht22 = 32;
const uint32_t pin_buttn = 0;
const uint32_t pin_esp_mode = 17;
bool __host;


void setup()
{
    pinMode(pin_esp_mode, INPUT_PULLUP);
    delay(1000);
    __host = digitalRead(pin_esp_mode) != HIGH;

    mprint("Starting in 1.5 sec...\n");
    if (__host) {mprint("======== HOST MODE ========\n");}
    else        {mprint("======== CLIENT MODE ========\n");}

    delay(1500);
    
    mprint("Configuring and spawning threads...\n");

    if (__host) dht_begin(pin_dht22);
    if (!__host) battery_begin();
    button_begin(pin_buttn);
    display_begin(__host);
    lora_begin(__host);

    if (!__host) {
        request_model mod;
        mod.request_type = 1; // UPDATE
        __lora_send((char*)&mod, sizeof(mod));
    }

    //xTaskCreatePinnedToCore(task_measure, "measurer", 4096, nullptr, 500, &thr_measurer, 1);
}

void loop() { vTaskDelete(NULL); }

//void task_measure(void*)
//void loop()
//{
//    mprint("[Loop] Task measurer up!\n");
//    delay(2000);
//    while(1) {
//        float t, u;
//        dht_read(t, u);
//        calc_call(t, u);
//        //const float prec = calc_call(t, u);
//        //Serial.printf("Read: %.2f C | %.2f %% | P=%.2f\n", t, 100.0f * u, prec);
//        yield();
//        delay(2000);
//    }
//}