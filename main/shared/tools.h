#pragma once

extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "driver/gpio.h"
    #include "sdkconfig.h"
    #include "esp_log.h"

    #define millis() (esp_timer_get_time() / 1000ULL)
    #define delay(MILLISECONDS) vTaskDelay(MILLISECONDS / portTICK_PERIOD_MS)
    #define delay_us(MICROSEC) ets_delay_us(MICROSEC)
    
}

#define cpu_seconds() get_time_copy().low

#define spin_on_fail(TAG, TEST, MESSAGE) if (!(TEST)) { ESP_LOGE(TAG, MESSAGE); while(1) {delay(250);}}
#define insert_format_into(STRING, FORMAT, ...) { const size_t act = STRING.size(); STRING.resize(act + snprintf(nullptr, 0, FORMAT, __VA_ARGS__) + 1); snprintf(STRING.data() + act, STRING.size() - act, FORMAT, __VA_ARGS__); }

// default stuff on all cases

constexpr int64_t timeout_screen_on = 120; // s
constexpr int64_t move_screen_sleep = 30; // s

#define MEGASMOL u8g2_font_4x6_tr
constexpr uint16_t font_height_megasmol = 5;

#define SMOLITALIC u8g2_font_prospero_bold_nbp_tf //u8g2_font_7x13B_tf
#define MIDDEFAULT u8g2_font_fub11_tf  //u8g2_font_crox3hb_tf 
constexpr gpio_num_t button_pin = GPIO_NUM_0;
constexpr gpio_num_t led_pin = GPIO_NUM_25;

// LoRa
#define PIN_NUM_MISO 	19
#define PIN_NUM_MOSI 	27
#define PIN_NUM_CLK  	5
#define PIN_NUM_CS   	18
#define PIN_NUM_DIO		26
#define RESET_PIN  		14
#define SENDER_RECEIVER_PIN	12