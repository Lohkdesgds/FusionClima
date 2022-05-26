#include <Arduino.h>
#include <heltec.h>
#include <thread>

#include "cpu_taskmanager.h"
#include "lora.h"
#include "sensoring_control.h"
#include "display.h"

constexpr uint16_t system_key = 0x8A;
constexpr int32_t ESP_MODE_PIN = 17;
constexpr int32_t ESP_LED = 25;

int64_t testt = 0;
uint64_t taskcounter = 0;
bool returned_once = true;
bool is_host_sensoring = false;

static void do_smth() 
{
    Serial.printf("= = = = = = = = = = = = = = = = = = = = Tasking @ %d...\n", xPortGetCoreID());
    char buf[256];
    for(size_t p = 0; p < 10000; ++p) {
        size_t cpy = p;
        for(int a = 255; a != 1; --a) {
            buf[a] = (cpy % 15) + '0';
            cpy /= 3;
            buf[a-1] = powf(cpy, 2.543f) * 1.03f - 4.f;
            buf[rand()%256] = buf[rand()%256];
        }
    }
    Serial.printf("= = = = = = = = = = = = = = = = = = = = End task @ %d\n", xPortGetCoreID());
}

void setup()
{
    setup_cpu_tools(false);
    pinMode(ESP_MODE_PIN, INPUT_PULLUP);
    pinMode(ESP_LED, OUTPUT);
    
    is_host_sensoring = digitalRead(ESP_MODE_PIN) != HIGH;// ? ESP_MODE::SENSOR : ESP_MODE::LISTENER;
    
    Serial.begin(115200);
    
    digitalWrite(ESP_LED, HIGH);
    delay(500);
    while(!Serial) delay(100);
    digitalWrite(ESP_LED, LOW);

    LR.begin(system_key);
    //lora_hook_recv([](pack& p){  Serial.printf("GOOD: %s\n", p.data.data()); returned_once = true; });

    delay(random(500, 2000));

    Serial.println("Setup end! All good!");
    
    if (is_host_sensoring) {
        Serial.println("Mode selected: SENSOR/HOST");
        
        TimedTasker.add([&]{
            auto pak = LR.pop();
            if (pak && pak.ptr_len == sizeof(Protocol::prot_bomb)) {
                const Protocol::prot_bomb& pb = (*(Protocol::prot_bomb*)pak.ptr.get());
                switch(pb.type) {
                case Protocol::prot_type::REQUEST_INFO:
                {
                    if (!Syncer.host_manual_post()) Serial.println("Failed to manually post requested data");
                    else Serial.println("One user requested info, manual post done.");
                }
                    break;
                default:
                    break;
                }
            }
            Syncer.host_auto_post();
        }, 5500, taskcounter++);
    }
    else {
        Serial.println("Mode selected: CLIENT");
        
        TimedTasker.add([&]{
            auto pak = LR.pop();
            if (pak && pak.ptr_len == sizeof(Protocol::prot_bomb)) {                
                const Protocol::prot_bomb& pb = (*(Protocol::prot_bomb*)pak.ptr.get());
                switch(pb.type) {
                case Protocol::prot_type::UPDATE:
                {
                    Serial.println("Got updated data");
                    Syncer.update(pb.data.forecast, true);
                }
                    break;
                default:
                    break;
                }
            }            
            Syncer.client_auto_request();
        }, 5000, taskcounter++);
    }
    
    TimedTasker.add([]{Syncer.any_ping(); Serial.println("PING (so signal update)");}, 30000, taskcounter++);
    TimedTasker.add([]{display.draw();}, 1000, taskcounter++);
    
    
    //TimedTasker.add([&]{
    //    Serial.printf("Hello, timed, @ %i! [%.2f%%, %.2f%%] --- PERC: %.1f%% DISPLAY, %.1f%% TEXT\n", 
    //    get_cpu_clock(), get_cpu_usage_id(0) * 100.0f, get_cpu_usage_id(1) * 100.0f, 100.0f * TimedTasker.get_cpu_time_of(1), 100.0f * TimedTasker.get_cpu_time_of(2));
    //}, 5000, taskcounter++);

    /*TimedTasker.add([]{    

    }, 10000, taskcounter++);*/

    //
    //TimedTasker.add([]{pack p = LR.pop(); if (p) {Serial.printf("GOOD: %s\n", p.ptr.get()); } else { Serial.println("No pop available"); } }, 2500, taskcounter++);
    //TimedTasker.add([]{Serial.printf("Hello, TimedTasker, @ %i!\n", get_cpu_clock());}, 2300, taskcounter++);
    /*TimedTasker.add([]{Serial.printf("====================\n"
                             "[#0]: %.2f%%\n"
                             "[#1]: %.2f%%\n"
                             "Total CPU: %.2f%%\n"
                             "Queue size: %zu\n"
                             "Clock CPU: %i MHz\n"
                             "RAM free: %zu of %zu (%.2f%%)\n"
                             "====================\n", 
                             get_cpu_usage_id(0) * 100.0f, get_cpu_usage_id(1) * 100.0f, get_cpu_usage() * 100.0f,
                             Tasker.queue_size(), getCpuFrequencyMhz(),
                             get_ram_free_bytes(), get_ram_total_bytes(), 100.0f * get_ram_usage()
                             );}, 3000, taskcounter++);*/

    /*delay(10000);
    for(int a = 0; a < 4; ++a) {
        delay(100);
        TimedTasker.add([]{do_smth();}, 60000, taskcounter++);
    }*/
}

/*void loop()
{
    delay(random(2000,3000));
    while(1) {
        for (size_t p = 0; p < 4 && !LR.has(); ++p) delay(1000);
        auto pk = LR.pop();
        if (pk) {
            Serial.printf("PK! %s\n", pk.ptr.get());
        }
        else break;
    }
    char test[64]{};
    sprintf(test, "%lli", ++testt);
    Serial.println("Sending...");
    if (!LR.send(test, 64)) Serial.println("Failed");
    else Serial.println("Sended successfully.");
    //returned_once = false;
    //char test[64]{};
    //size_t len = sprintf(test, "%lli", ++testt);
    //Serial.println("Sending...");
    //if (!LR.send(test, 64)) Serial.println("Failed");
    //else Serial.println("Sended successfully.");
    //delay(7000);
}*/
void loop() { vTaskDelete(NULL); } // remove loop entirely
