#include "displaying.h"

U8G2_SSD1306_128X64_NONAME_F_SW_I2C* __dsp = nullptr;

void display_begin(bool host)
{
    __dsp = new U8G2_SSD1306_128X64_NONAME_F_SW_I2C(U8G2_R0, DISP_CLK_PIN, DISP_DATA_PIN, DISP_RST_PIN);
    __dsp->begin();
    __dsp->enableUTF8Print();
    if (host) xTaskCreatePinnedToCore(__display_update_host,   "AsyncMonH", 3072, nullptr, tskIDLE_PRIORITY, nullptr, 0);
    else      xTaskCreatePinnedToCore(__display_update_client, "AsyncMonC", 3072, nullptr, tskIDLE_PRIORITY, nullptr, 0);
}

void __display_update_client(void*)
{
    mprint("[Display] Starting to draw indefinitely...\n");
    esp_task_wdt_init(30, false);
    char dmp[96];
    while(1) {
        //__dsp->setFont(u8g2_font_tiny_simon_tr);
        __dsp->setFont(u8g2_font_pixzillav1_tf);

        sprintf(dmp, u8"%04.1fºC, %.1f%% %.1f%%", __rawshrdata.m_temp, 100.0f * __rawshrdata.m_umid, 100.0f * __rawshrdata.m_battlevel);
        __dsp->drawUTF8(0, 10, dmp);

        sprintf(dmp, u8"Chuva: %.2f%%", 100.0f * __rawshrdata.m_rainchance);
        __dsp->drawUTF8(0, 25, dmp);
        
        sprintf(dmp, u8"Último: %llu s", (TIME_NOW_MS - __rawshrdata.m_last_send) / 1000);
        __dsp->drawUTF8(0, 40, dmp);

        const uint8_t limit_y = 42;
        const uint8_t limit_max_y = 63;

        for(uint8_t p = 0; p < data_graph_size_fixed; ++p) {
            __dsp->drawPixel(p, limit_max_y - ((limit_max_y - limit_y) * __rawshrdata.conv_data_pt_rain(p)));
        }

        //sprintf(dmp, "%.1fC %.1f%% %.1f%%", __rawshrdata.m_temp, 100.0f * __rawshrdata.m_umid, 100.0f * __rawshrdata.m_rainchance);        
        //__dsp->drawUTF8(0, 5, dmp);

        __dsp->sendBuffer();
        __dsp->clearBuffer();

        delay(100);
    }
}

void __display_update_host(void*)
{
    mprint("[Display] Starting to draw indefinitely...\n");
    esp_task_wdt_init(30, false);
    char dmp[96];
    while(1) {
        //__dsp->setFont(u8g2_font_tiny_simon_tr);
        __dsp->setFont(u8g2_font_pixzillav1_tf);

        {
            sprintf(dmp, u8"INST: %04.1fºC %.1f%% | MM: %04.1fºC %.1f%%", __rawshrdata.m_temp, 100.0f * __rawshrdata.m_umid, __rawshrdata.m_temp_avg, 100.0f * __rawshrdata.m_umid_avg);
            const uint32_t rawlen = __dsp->getUTF8Width(dmp);
            const uint32_t exceed = rawlen > DISP_WIDTH ? (rawlen - DISP_WIDTH) : 0;
            const uint32_t limits_cte = 30;

            const uint32_t offx = ((TIME_NOW_MS / 50) % (exceed + (2 * limits_cte)));

            __dsp->drawUTF8(-(offx < limits_cte ? 0 : (offx > (exceed + limits_cte) ? exceed : (offx - limits_cte))), 10, dmp);
        }
                
        sprintf(dmp, u8"Chuva: %.2f%%", 100.0f * __rawshrdata.m_rainchance);
        __dsp->drawUTF8(0, 25, dmp);
        
        sprintf(dmp, u8"Último: %llu s", (TIME_NOW_MS - __rawshrdata.m_last_send) / 1000);
        __dsp->drawUTF8(0, 40, dmp);

        const uint8_t limit_y = 42;
        const uint8_t limit_max_y = 63;

        for(uint8_t p = 0; p < data_graph_size_fixed; ++p) {
            __dsp->drawPixel(p, limit_max_y - ((limit_max_y - limit_y) * __rawshrdata.conv_data_pt_rain(p)));
        }

        //sprintf(dmp, "%.1fC %.1f%% %.1f%%", __rawshrdata.m_temp, 100.0f * __rawshrdata.m_umid, 100.0f * __rawshrdata.m_rainchance);        
        //__dsp->drawUTF8(0, 5, dmp);

        __dsp->sendBuffer();
        __dsp->clearBuffer();

        delay(100);
    }
}