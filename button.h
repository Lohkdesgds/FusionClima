#pragma once

#include <heltec.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_freertos_hooks.h>
#include <esp_task_wdt.h>

#include "debug_wrapper.h"
#include "cpuctl.h"

extern uint32_t __button_wakeup;

void button_begin(const uint32_t btn);

void IRAM_ATTR _sleep_auto();
void __detach_sleep_async(void*);
void IRAM_ATTR __button_interrupt();