#pragma once

extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "driver/gpio.h"
    #include "sdkconfig.h"
    #include "esp_log.h"

    #define millis() (esp_timer_get_time() / 1000ULL)
    #define delay(MILLISECONDS) vTaskDelay(MILLISECONDS / portTICK_PERIOD_MS)
    #define delay_us(MICROSEC) ets_delay_us(MICROSEC);
    
}

#define spin_on_fail(TAG, TEST, MESSAGE) if (!(TEST)) { ESP_LOGE(TAG, MESSAGE); while(1) {delay(250);}}

// default stuff on all cases

constexpr int64_t timeout_screen_on = 120ULL * 1000000ULL; // us
constexpr int64_t move_screen_sleep = 30ULL * 1000000ULL; // us

#define MEGASMOL u8g2_font_4x6_tr
constexpr uint16_t font_height_megasmol = 5;

#define SMOLITALIC u8g2_font_prospero_bold_nbp_tf //u8g2_font_7x13B_tf
#define MIDDEFAULT u8g2_font_fub11_tf  //u8g2_font_crox3hb_tf 
constexpr gpio_num_t button_pin = GPIO_NUM_0;
constexpr gpio_num_t led_pin = GPIO_NUM_25;