#include "button.h"

uint32_t __button_wakeup = 0;

void button_begin(const uint32_t btn)
{
    __button_wakeup = btn;
    touchAttachInterrupt(__button_wakeup, _sleep_auto, 1);
}

void IRAM_ATTR _sleep_auto() // low
{
    touchAttachInterrupt(__button_wakeup, NULL, 0); // detach itself
    mprint("[BUTTON] Triggered @ %llu ms! Sleeping soon...\n", TIME_NOW_MS);
    xTaskCreate(__detach_sleep_async, "AsyncSleepCall", 2048, nullptr, 10, nullptr);
}

void __detach_sleep_async(void*)
{
    //Serial.println("Sleeping in 2 seconds...");
    delay(1000);
    //Serial.println("Sleeping ESP32...");

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_OFF);
    //esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_OFF);

    touchAttachInterrupt(__button_wakeup, __button_interrupt, 1);
    esp_sleep_enable_touchpad_wakeup();
    //esp_set_deep_sleep_wake_stub();

    esp_deep_sleep_start();
}

void IRAM_ATTR __button_interrupt() {}