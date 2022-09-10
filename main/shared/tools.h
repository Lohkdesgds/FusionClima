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