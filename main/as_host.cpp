#include "shared/tools.h"
#include "shared/buttons.h"
#include "shared/display.h"
#include "shared/import/DHT.h"
#include "shared/wifictl.h"

enum class host_menus{ QRCODE, TEMPERATURE, LORA, NONE, _MAX };

const char TAG[] = "MAINHST";
host_menus _menu = host_menus::QRCODE;
qrcodegen::QrCode* qr = nullptr;
DHT* hdt = nullptr;
int64_t last_click = 0;

constexpr int64_t timeout_screen_on = 20ULL * 1000000ULL; // us
#define SMOLITALIC u8g2_font_prospero_bold_nbp_tf //u8g2_font_7x13B_tf
#define MIDDEFAULT u8g2_font_fub11_tf  //u8g2_font_crox3hb_tf 
constexpr gpio_num_t button_pin = GPIO_NUM_0;
constexpr gpio_num_t led_pin = GPIO_NUM_25;

void handle_click(bool);
void async_update(void*);
void advance_menu();
std::string random_wifi_name();
std::string random_wifi_password();

void as_host()
{
    ESP_LOGI(TAG, "Started as host");

    ESP_LOGI(TAG, "Enabling LED for boot feedback");
    spin_on_fail(TAG, custom_gpio_setup(gpio_config_custom().set_pin(led_pin).set_mode(GPIO_MODE_OUTPUT)), "Cannot setup LED");
    custom_digital_write(led_pin, true);

    ESP_LOGI(TAG, "Enabling DHT readings");
    hdt = new DHT(GPIO_NUM_32);
    hdt->update();
    
    ESP_LOGI(TAG, "Waking up WiFi");
    custom_wifi_setup(random_wifi_name(), random_wifi_password());    

    ESP_LOGI(TAG, "Generating QR");
    qr = new qrcodegen::QrCode(custom_wifi_gen_QR());
    
    ESP_LOGI(TAG, "Working on button");
    spin_on_fail(TAG, custom_gpio_setup(gpio_config_custom().set_pin(button_pin).set_trigger(GPIO_INTR_NEGEDGE).set_mode(GPIO_MODE_INPUT)), "Cannot setup button #0");
    custom_gpio_map_to_function(button_pin, handle_click);
    custom_enable_gpio_functional(true);

    ESP_LOGI(TAG, "Starting async display");
    u8g2.setup(GPIO_NUM_4, GPIO_NUM_15, GPIO_NUM_16);
    xTaskCreatePinnedToCore(async_update, "dispasync", 3072, nullptr, 4, nullptr, 1);
    custom_digital_write(led_pin, false);
    
    ESP_LOGI(TAG, "Ready.");

    while(1) {
        delay(2000);
        hdt->update();
        ESP_LOGI(TAG, "Temp: %.1f; Hum: %.1f", hdt->getTemperature(), hdt->getHumidity());
    }
}

void handle_click(bool on)
{
    ESP_LOGI(TAG, "Switch triggered");
    advance_menu();
}

void async_update(void* unnused) {

    const bool do_double = qr->getSize() <= 32;
    const uint32_t offf = ((64 - static_cast<uint32_t>((do_double ? 2 : 1) * qr->getSize())) / 2);
    const uint32_t offtxt = offf + (qr->getSize() * (do_double ? 2 : 1));
    host_menus old_menu = _menu;
    last_click = esp_timer_get_time();

    while(1) {
        if (esp_timer_get_time() - last_click > 20000000){
            _menu = host_menus::NONE;
            last_click = esp_timer_get_time();
        }
        old_menu = _menu;

        switch(_menu) {
        case host_menus::QRCODE:
        {
            if (do_double) {
                for(int y = 0; y < qr->getSize(); ++y) {
                    for(int x = 0; x < qr->getSize(); ++x) {
                        if (qr->getModule(x, y)) {
                            u8g2.DrawBox(offf + 2 * x, offf + 2 * y, 2, 2);
                        }
                    }
                }
            }
            else {
                for(int y = 0; y < qr->getSize(); ++y) {
                    for(int x = 0; x < qr->getSize(); ++x) {
                        if (qr->getModule(x, y)) {
                            u8g2.DrawPixel(offf + x, offf + y);
                        }
                    }
                }
            }

            u8g2.SetFont(MIDDEFAULT);
            u8g2.DrawStr(offtxt + 3, 16, "SSID:");
            u8g2.SetFont(SMOLITALIC);
            u8g2.DrawStr(offtxt + 3, 29, custom_wifi_get_ssid().c_str());
            
            u8g2.SetFont(MIDDEFAULT);
            u8g2.DrawStr(offtxt + 3, 47, "Senha:");
            u8g2.SetFont(SMOLITALIC);
            u8g2.DrawStr(offtxt + 3, 60, custom_wifi_get_password().c_str());
        }
            break;
        case host_menus::TEMPERATURE:
        {
            char buff[32];

            u8g2.SetFont(MIDDEFAULT);
            u8g2.DrawStr(0, 16, "Temperatura:");
            sprintf(buff, u8"-> %.2f ºC", hdt->getTemperature());
            u8g2.SetFont(SMOLITALIC);
            u8g2.DrawUTF8(0, 29, buff);

            u8g2.SetFont(MIDDEFAULT);
            u8g2.DrawStr(0, 47, "Umidade:");
            sprintf(buff, u8"-> %.2f%%", hdt->getHumidity());
            u8g2.SetFont(SMOLITALIC);
            u8g2.DrawUTF8(0, 60, buff);
        }
            break;
        case host_menus::LORA:
            u8g2.SetFont(MIDDEFAULT);
            u8g2.DrawStr(0, 16, "LORA");
            break;
        case host_menus::NONE:
            custom_digital_write(led_pin, true);
            delay(10);
            custom_digital_write(led_pin, false);
            break;
        default:
            delay(100);
            custom_digital_write(led_pin, true);
            delay(100);
            custom_digital_write(led_pin, false);
            break;
        }
        
        u8g2.SendBuffer();
        for(size_t c = 0; c < 50 && old_menu == host_menus::NONE && _menu == host_menus::NONE; ++c) delay(50);
        delay(25);
        u8g2.ClearBuffer();
    }
}

void advance_menu() {
    last_click = esp_timer_get_time();
    _menu = static_cast<host_menus>(((static_cast<uint32_t>(_menu)) + 1) % static_cast<size_t>(host_menus::_MAX));
}

std::string random_wifi_name()
{
    std::string res;

    if (esp_random() % 2 == 0) res += "fus";
    else res += "cli";

    for(size_t rc = 0; rc < 6; ++rc) res += (esp_random() % 2 == 0) ? ('A' + (esp_random() % 26)) : ('a' + (esp_random() % 26));

    return res;
}

std::string random_wifi_password()
{
    std::string res;

    for(size_t rc = 0; rc < 8; ++rc) {
        switch(esp_random() % 3) {
        case 0: res += ('A' + (esp_random() % 26)); break;
        case 1: res += ('a' + (esp_random() % 26)); break;
        case 2: res += ('0' + (esp_random() % 10)); break;
        }
    }

    return res;
}