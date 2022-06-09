#pragma once

#include <heltec.h>
#include <chrono>
#include <mutex>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_freertos_hooks.h>
#include <esp_task_wdt.h>

struct __tools{
    std::mutex prt;

    __tools();
};
extern __tools __t;

#define TIME_NOW_MS std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::milli>>(std::chrono::system_clock::now().time_since_epoch()).count()
#define mprint(...) { std::lock_guard<std::mutex> l(__t.prt); Serial.printf(__VA_ARGS__); }