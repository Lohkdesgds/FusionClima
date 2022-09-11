#pragma once

extern "C" {
    #include <string.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "esp_system.h"
    #include "esp_wifi.h"
    #include "esp_event.h"
    #include "esp_log.h"
    #include "nvs_flash.h"
}

#include "import/qrcodegen.hpp" // Source: https://github.com/nayuki/QR-Code-generator/tree/master/cpp (thank you)

#include <string>

struct wifi_info {
    const std::string* html_followup = nullptr; // avoid copy
    bool event_active = false;
    uint8_t connected_devices = 0;
    std::string qrcoder;
    std::string ssid_cpy, pw_cpy;
};

void custom_wifi_setup(const std::string& ssid, const std::string& pw, const uint8_t maxdevices = 4, uint8_t chh = 13);
void custom_wifi_stop();
void custom_wifi_setup_homepage(std::string);
qrcodegen::QrCode custom_wifi_gen_QR();
const std::string& custom_wifi_get_ssid();
const std::string& custom_wifi_get_password();

void __custom_wifi_handler(void*, esp_event_base_t, int32_t, void*);