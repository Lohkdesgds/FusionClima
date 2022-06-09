#pragma once

#include <heltec.h>
#include <memory>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_freertos_hooks.h>
#include <esp_task_wdt.h>

#include "tools.h"
#include "data.h"

struct lora_dgram {
    std::unique_ptr<char[]> ptr;
    uint32_t ptr_len = 0;
    //int32_t rssi = 0;
    //float snr = 0.0f;
};

struct lora_fire {
    lora_dgram ptr;
    std::atomic<bool> has_ptr;

    lora_fire();
    bool post(lora_dgram&&);
    lora_dgram wait();
    void free();
};

extern uint16_t __loracode;
extern std::mutex __lorasnd;
extern lora_fire __lorafire;

void __lorahook(int);

bool lora_begin(bool host, long freq = 915E6, uint16_t code = 0xFA);
bool __lora_send(char* buf, size_t len);

void __lora_recv_client(void*);
void __lora_recv_host(void*);

void __lora_loop_host(void*);
