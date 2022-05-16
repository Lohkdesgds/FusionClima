#include <Arduino.h>
#include <heltec.h>
#include <thread>

#include "cpu_taskmanager.h"
#include "cpu_timed.h"
#include "display.h"

TaskManager tasker;
TimedAction timed(tasker);
Displayer disp;

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
  Serial.begin(115200);
  while(!Serial) delay(100);
    
  Serial.println("Setup end! All good!");


  timed.add([]{disp.draw();}, 2000, 1);
  //timed.add([]{Serial.printf("Hello, timed, @ %i!\n", get_cpu_clock());}, 2300);
  timed.add([&]{Serial.printf("Hello, timed, @ %i! [%.2f%%, %.2f%%] --- PERC: %.1f%% DISPLAY, %.1f%% TEXT\n", get_cpu_clock(), get_cpu_usage_id(0) * 100.0f, get_cpu_usage_id(1) * 100.0f, 100.0f * timed.get_cpu_time_of(1), 100.0f * timed.get_cpu_time_of(2));}, 5000, 2);
  /*timed.add([]{Serial.printf("====================\n"
                             "[#0]: %.2f%%\n"
                             "[#1]: %.2f%%\n"
                             "Total CPU: %.2f%%\n"
                             "Queue size: %zu\n"
                             "Clock CPU: %i MHz\n"
                             "RAM free: %zu of %zu (%.2f%%)\n"
                             "====================\n", 
                             get_cpu_usage_id(0) * 100.0f, get_cpu_usage_id(1) * 100.0f, get_cpu_usage() * 100.0f,
                             tasker.queue_size(), getCpuFrequencyMhz(),
                             get_ram_free_bytes(), get_ram_total_bytes(), 100.0f * get_ram_usage()
                             );}, 3000);*/

  delay(10000);
  for(int a = 0; a < 4; ++a) {
    delay(100);
    timed.add([]{do_smth();}, 60000, 0);
  }
}

void loop() { vTaskDelete(NULL); } // remove loop entirely
