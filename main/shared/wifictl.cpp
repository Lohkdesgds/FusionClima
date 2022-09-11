#include "wifictl.h"

const char *TAG = "wifi softAP";
wifi_info _wifi;
const std::string index_html = "<!DOCTYPE html><html><head><title>ESP32 not configured</title></head><body><p>Please do custom_wifi_homepage().</p></body></html>";

void custom_wifi_setup(const std::string& sss, const std::string& pw, const uint8_t maxdevices, uint8_t chh)
{
    if (_wifi.event_active) custom_wifi_stop();

    if (!_wifi.html_followup) _wifi.html_followup = &index_html;

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &__custom_wifi_handler,
                                                        NULL,
                                                        NULL));

    // [1..13]
    if (chh > 13) chh = 13;
    if (chh < 1) chh = 1;

    wifi_config_t wifi_config;
    
    //wifi_config.ap.ssid = {};
    //wifi_config.ap.ssid_len = 0;
    wifi_config.ap.channel = chh;
    //wifi_config.ap.password = {};
    wifi_config.ap.max_connection = maxdevices;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    _wifi.ssid_cpy = sss.substr(0, std::size(wifi_config.ap.ssid) - 1);
    _wifi.pw_cpy = pw.substr(0, std::size(wifi_config.ap.password) - 1);

    sprintf((char*)wifi_config.ap.ssid, "%s", _wifi.ssid_cpy.c_str());
    wifi_config.ap.ssid_len = _wifi.ssid_cpy.size();

    if (pw.size()) {
        sprintf((char*)wifi_config.ap.password, "%s", _wifi.pw_cpy.c_str());
    }
    else {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             wifi_config.ap.ssid, wifi_config.ap.password, wifi_config.ap.channel);

    _wifi.qrcoder = "WIFI:T:WPA;S:" + _wifi.ssid_cpy + ";P:" + _wifi.pw_cpy + ";H:false;";
    _wifi.event_active = true;
}

void custom_wifi_stop()
{
    if (!_wifi.event_active) return; 
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_netif_deinit());
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
    ESP_ERROR_CHECK(nvs_flash_deinit());
    _wifi.event_active = false;
}

void custom_wifi_setup_homepage(const std::string* replace)
{
    _wifi.html_followup = replace;
}

qrcodegen::QrCode custom_wifi_gen_QR()
{
    return qrcodegen::QrCode::encodeText(_wifi.qrcoder.c_str(), qrcodegen::QrCode::Ecc::LOW);
}

const std::string& custom_wifi_get_ssid()
{
    return _wifi.ssid_cpy;
}

const std::string& custom_wifi_get_password()
{
    return _wifi.pw_cpy;
}

void __custom_wifi_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    switch(event_id) {
    case WIFI_EVENT_AP_STACONNECTED:
    {
        ++_wifi.connected_devices;
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    }
        break;
    case WIFI_EVENT_AP_STADISCONNECTED:
    {
        --_wifi.connected_devices;
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
        break;
    default:
        ESP_LOGI(TAG, "station unhandled event (not error)");
        break;
    }
}