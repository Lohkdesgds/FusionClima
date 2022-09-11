#include "wifictl.h"

esp_err_t root_get_handler(httpd_req_t*);

const char *TAG = "WIFI";
wifi_info _wifi;
extern const char root_start[] asm("_binary_root_html_start");
extern const char root_end[] asm("_binary_root_html_end");
const uint32_t root_len = root_end - root_start;
std::vector<std::pair<wifi_pair, wifi_handler>> hooks_wifi;
//const httpd_uri_t _webroot = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler, .user_ctx = nullptr };


bool wifi_pair::operator==(const wifi_pair& oth) const
{
    return oth.path == path && oth.method == method;
}

bool wifi_pair::operator!=(const wifi_pair& oth) const
{
    return oth.path != path || oth.method != method;
}

// HTTP GET Handler
esp_err_t root_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Serve root");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, root_start, root_len);
    return ESP_OK;
}
// HTTP Error (404) Handler - Redirects all requests to the root page
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    // Set status
    httpd_resp_set_status(req, "302 Temporary Redirect");
    // Redirect to the "/" root directory
    httpd_resp_set_hdr(req, "Location", "/");
    // iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
    httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Redirecting to root");
    return ESP_OK;
}

void custom_wifi_setup(const std::string& sss, const std::string& pw, const bool autohttpfp, const uint8_t maxdevices, uint8_t chh)
{
    if (_wifi.event_active) custom_wifi_stop();

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

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_open_sockets = 7;
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting webserver on port: '%d'", config.server_port);
    if (httpd_start(&_wifi.webserv, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        //httpd_register_uri_handler(_wifi.webserv, &_webroot);
        if (autohttpfp) custom_wifi_add_handle("/", HTTP_GET, root_get_handler);
        httpd_register_err_handler(_wifi.webserv, HTTPD_404_NOT_FOUND, http_404_error_handler);
    }
    
    start_dns_server();

    _wifi.qrcoder = "WIFI:T:WPA;S:" + _wifi.ssid_cpy + ";P:" + _wifi.pw_cpy + ";H:false;";
    _wifi.event_active = true;
}

bool custom_wifi_add_handle(std::string path, httpd_method_t met, esp_err_t (*handler)(httpd_req_t*), void* usrdata)
{
    wifi_pair gen = { .path = std::move(path), .method = met };

    custom_wifi_del_handle(gen.path, gen.method);

    auto iit = hooks_wifi.insert(hooks_wifi.end(), std::pair<wifi_pair, wifi_handler>{ gen, wifi_handler{} }); // iterator

    //httpd_uri_t handl;// = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler, .user_ctx = nullptr };
    iit->second.handl.uri = iit->first.path.c_str();
    iit->second.handl.method = met;
    iit->second.handl.handler = handler;
    iit->second.handl.user_ctx = usrdata;
    return httpd_register_uri_handler(_wifi.webserv, &iit->second.handl) == ESP_OK;
}

void custom_wifi_del_handle(std::string path, httpd_method_t met)
{
    wifi_pair gen = { .path = std::move(path), .method = met };

    if (auto it = std::find_if(hooks_wifi.begin(), hooks_wifi.end(), [&](const std::pair<wifi_pair, wifi_handler>& p) { return p.first == gen; }); it != hooks_wifi.end()) {
        httpd_unregister_uri_handler(_wifi.webserv, gen.path.c_str(), gen.method);
        hooks_wifi.erase(it);
    }
}

void custom_wifi_stop()
{
    if (!_wifi.event_active) return; 
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_netif_deinit());
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
    ESP_ERROR_CHECK(nvs_flash_deinit());
    _wifi.connected_devices = 0;
    _wifi.event_active = false;
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

const uint8_t& custom_wifi_get_count()
{
    return _wifi.connected_devices;
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