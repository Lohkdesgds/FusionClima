#pragma once

extern "C" {
    #include <string.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "esp_system.h"
    #include "esp_event.h"
    #include "esp_log.h"

    #include "nvs_flash.h"
    #include "esp_wifi.h"
    #include "esp_netif.h"
    #include "lwip/inet.h"

    #include "esp_http_server.h"
    #include "import/dns_server.h" // Source: https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/captive_portal/main (ty)
}

#include "import/qrcodegen.hpp" // Source: https://github.com/nayuki/QR-Code-generator/tree/master/cpp (thank you)

#include <string>
#include <vector>
#include <algorithm>

struct wifi_info {
    bool event_active = false;
    uint8_t connected_devices = 0;
    std::string qrcoder;
    std::string ssid_cpy, pw_cpy;
    httpd_handle_t webserv = nullptr;
};

struct wifi_handler {
    httpd_uri_t handl = { .uri = "/", .method = HTTP_GET, .handler = nullptr, .user_ctx = nullptr };
};

struct wifi_pair {
    std::string path{};
    httpd_method_t method = HTTP_GET;

    wifi_pair() = default;
    bool operator==(const wifi_pair&) const;
    bool operator!=(const wifi_pair&) const;
};

void custom_wifi_setup(const std::string& ssid, const std::string& pw, const bool autohttpfp = true, const uint8_t maxdevices = 4, uint8_t chh = 13);
// optional
bool custom_wifi_add_handle(std::string path, httpd_method_t met, esp_err_t (*handler)(httpd_req_t*), void* usrdata = nullptr);
// optional
void custom_wifi_del_handle(std::string path, httpd_method_t met);
void custom_wifi_stop();
qrcodegen::QrCode custom_wifi_gen_QR();
const std::string& custom_wifi_get_ssid();
const std::string& custom_wifi_get_password();
const uint8_t& custom_wifi_get_count();

void __custom_wifi_handler(void*, esp_event_base_t, int32_t, void*);