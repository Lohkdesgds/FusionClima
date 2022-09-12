#include "main_shared.h"

void handle_click(bool);
void async_update(void*);
void advance_menu();
std::string random_wifi_name();
std::string random_wifi_password();

void as_host()
{
    ESP_LOGI(HOSTTAG, "Started as host");

    ESP_LOGI(HOSTTAG, "Enabling LED for boot feedback");
    spin_on_fail(HOSTTAG, custom_gpio_setup(gpio_config_custom().set_pin(led_pin).set_mode(GPIO_MODE_OUTPUT)), "Cannot setup LED");
    custom_digital_write(led_pin, true);
    
    ESP_LOGI(HOSTTAG, "Preparing LoRa.");    
	lora = new LoRa( PIN_NUM_MOSI, PIN_NUM_MISO, PIN_NUM_CLK, PIN_NUM_CS, RESET_PIN, PIN_NUM_DIO, 10 );

    ESP_LOGI(HOSTTAG, "Enabling DHT readings");
    hdt = new DHT(GPIO_NUM_32);
    hdt->update();
    
    ESP_LOGI(HOSTTAG, "Waking up WiFi");
    custom_wifi_setup("potatoed", "arrozfeij"); //random_wifi_name(), random_wifi_password());    

    ESP_LOGI(HOSTTAG, "Generating QR");
    qr = new qrcodegen::QrCode(custom_wifi_gen_QR());
    
    ESP_LOGI(HOSTTAG, "Working on button");
    spin_on_fail(HOSTTAG, custom_gpio_setup(gpio_config_custom().set_pin(button_pin).set_trigger(GPIO_INTR_NEGEDGE).set_mode(GPIO_MODE_INPUT)), "Cannot setup button #0");
    custom_gpio_map_to_function(button_pin, handle_click);
    custom_enable_gpio_functional(true);

    ESP_LOGI(HOSTTAG, "Starting async display");
    u8g2.setup(GPIO_NUM_4, GPIO_NUM_15, GPIO_NUM_16);
    xTaskCreatePinnedToCore(async_update, "dispasync", 3072, nullptr, 4, nullptr, 1);
    custom_digital_write(led_pin, false);
    
    ESP_LOGI(HOSTTAG, "Ready.");

    while(1) {
        delay(10000);
        hdt->update();

        if (_menu != host_menus::NONE) custom_digital_write(led_pin, true);

        lora->beginPacket(false);
        lora->write(("Hello there fellow friend, weather here is " + std::to_string(hdt->getTemperature()) + " degrees celsius and " + std::to_string(hdt->getHumidity()) + " percent umid.").c_str());
	    lora->endPacket(false);

        delay(10);
        if (_menu != host_menus::NONE) custom_digital_write(led_pin, false);

        //ESP_LOGI(HOSTTAG, "Temp: %.1f; Hum: %.1f", hdt->getTemperature(), hdt->getHumidity());
    }
}

void handle_click(bool on)
{
    ESP_LOGI(HOSTTAG, "Switch triggered");
    advance_menu();
}

void async_update(void* unnused) {

    const bool do_double = qr->getSize() <= 32;
    const uint32_t offf = ((64 - static_cast<uint32_t>((do_double ? 2 : 1) * qr->getSize())) / 2);
    const uint32_t offtxt = offf + (qr->getSize() * (do_double ? 2 : 1));
    //host_menus old_menu = _menu;
    int64_t last_movement = 0, last_text_upd = 0;
    u8g2_uint_t none_p[2] = {0,0};
    u8g2_uint_t off_p[2] = {0,0};
    u8g2_uint_t l_max_x = 0;
    std::string line1, line2;

    last_click = cpu_seconds();

    while(1) {
        if (cpu_seconds() - last_click > timeout_screen_on){
            _menu = host_menus::NONE;
            last_click = cpu_seconds();
        }
        //old_menu = _menu;

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
            {
                u8g2.SetFont(MEGASMOL);
                if (auto _t = cpu_seconds(); _t != last_text_upd || last_text_upd == 0) {
                    last_text_upd = _t;

                    custom_digital_write(led_pin, true);
                    delay(10);
                    custom_digital_write(led_pin, false);

                    line1 = "WiFi: " + std::to_string(custom_wifi_get_count());
                    insert_format_into(line1, "; %.1f%% %.1f%%", 100.0f * get_cpu_usage_all(), 100.0f * get_ram_usage());
                    line2 = "Uptime: ";// + std::to_string(_t / 1000000ULL) + " s";

                    const int64_t sec = _t % 60;
                    _t -= sec;
                    const int64_t min_raw = _t % 3600; // /60 l8
                    _t -= min_raw;
                    const int64_t hour_raw = _t % (3600 * 24); // /3600 l8
                    _t -= hour_raw;
                    const int64_t days = _t / (3600 * 24);
                    
                    if (days > 0) line2 += std::to_string(days) + "d";
                    if (hour_raw > 0) line2 += std::to_string(hour_raw / 3600) + "h";
                    if (min_raw > 0) line2 += std::to_string(min_raw / 60) + "m";
                    line2 += std::to_string(sec) + (min_raw > 0 ? "s" : " sec");

                    const auto wl1 = u8g2.GetStrWidth(line1.c_str());
                    const auto wl2 = u8g2.GetStrWidth(line2.c_str());
                    off_p[0] = wl1 > wl2 ? 0 : ((wl2 - wl1) / 2);
                    off_p[1] = wl2 > wl1 ? 0 : ((wl1 - wl2) / 2);
                    l_max_x = 128 - std::max(wl1, wl2);
                }
                if (int64_t _t = cpu_seconds(); _t - last_movement > move_screen_sleep || last_movement == 0) {
                    last_movement = _t;

                    const u8g2_uint_t max_y = 64 - (2 * font_height_megasmol + 1);

                    none_p[0] = esp_random() % l_max_x;
                    none_p[1] = (esp_random() % max_y) + font_height_megasmol;
                }
                u8g2.DrawStr(none_p[0] + off_p[0], none_p[1], line1.c_str());
                u8g2.DrawStr(none_p[0] + off_p[1], none_p[1] + font_height_megasmol + 1, line2.c_str());
            }
            break;
        default:
            delay(100);
            custom_digital_write(led_pin, true);
            delay(100);
            custom_digital_write(led_pin, false);
            break;
        }
        
        u8g2.SendBuffer();
        //for(size_t c = 0; c < 18 && old_menu == host_menus::NONE && _menu == host_menus::NONE; ++c) delay(45);
        delay(25);
        u8g2.ClearBuffer();
    }
}

void advance_menu() {
    last_click = cpu_seconds();
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