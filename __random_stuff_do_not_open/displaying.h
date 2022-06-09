#pragma once

#include <U8g2lib.h>
#include <heltec.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_freertos_hooks.h>
#include <esp_task_wdt.h>

#include "data.h"
#include "tools.h"

constexpr uint32_t DISP_CLK_PIN     = 15;
constexpr uint32_t DISP_DATA_PIN    = 4;
constexpr uint32_t DISP_RST_PIN     = 16;
constexpr uint32_t DISP_WIDTH       = 128;
constexpr uint32_t DISP_HEIGHT      = 64;

extern U8G2_SSD1306_128X64_NONAME_F_SW_I2C* __dsp;

void display_begin(bool host);

void __display_update_client(void*);
void __display_update_host(void*);