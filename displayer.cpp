#include "displayer.h"

Displayer display;
    
void Displayer::draw_top()
{
    static char dmp[64];
    m_dsp->drawXBM(0, 0, 128, 6, u8g_topbar2);
    
    m_dsp->setFont(u8g2_font_tiny_simon_tr); 
    
    // CPU USAGE
    { const float _cpu = 99.9f * get_cpu_usage(); if (_cpu >= 10.0f) {sprintf(dmp, "%04.1f%%", _cpu);} else {sprintf(dmp, "%04.2f%%", _cpu);}}
    m_dsp->drawUTF8(0, 5, dmp); // CPU USAGE
    
    // RAM USAGE
    { const float _ram = 99.9f * get_ram_usage(); if (_ram >= 10.0f) {sprintf(dmp, "%04.1f%%", _ram);} else {sprintf(dmp, "%04.2f%%", _ram);}}
    //sprintf(dmp, "%04.1f%%", 99.9f * get_ram_usage());  
    m_dsp->drawUTF8(24, 5, dmp); 
    
    //////m_dsp->drawUTF8(43, 5, "88"); // DEVICES ECHOING (TRANSMITTERS RECEIVED) // REMOVED FROM FINAL

    //m_dsp->drawUTF8(56, 5, "88"); // TIME SINCE LAST RECV (minutes)
    { const unsigned long long dt = ((get_time_ms() - g_data.last_lora_actitivy) / 1000000) % 1000; sprintf(dmp, "%04llu", dt); }
    m_dsp->drawUTF8(48, 5, dmp); // TIME SINCE LAST RECV (minutes)
    
    // LAST BEST SIGNAL (-dB)
    //m_dsp->drawUTF8(68, 5, "188dB");
    { const int32_t _rssi = LR.currentRssi(); if (_rssi > 0) {sprintf(dmp, "00dB");} else if (_rssi > -199) {sprintf(dmp, "%02idB", -_rssi);} else {sprintf(dmp, "-199dB");} }
    m_dsp->drawUTF8(68, 5, dmp); // LAST BEST SIGNAL (-dB)
    
    //m_dsp->drawUTF8(113, 5, "188%%"); // BATTERY PERC [0..100]  
    {
        const float _perc = get_battery_perc(true);
        if (_perc > 0.99f)   sprintf(dmp, "100%%");
        else                 sprintf(dmp, " %02.0f%%", 100.0f * _perc);
        //Serial.printf("PERC DRAWN: %s\n", dmp);
        m_dsp->drawUTF8(113, 5, dmp);
        m_dsp->drawLine(109, 2, 109 - constrain(map((int)(_perc * 100.0f), 0, 100, 0, 5), 0, 4), 2);  // BATTERY BAR [109..105]
    }
    
    m_dsp->drawBox(89, 3, 2, 1); // LOW  BAR SIGNAL
    m_dsp->drawBox(92, 2, 2, 2); // ...  BAR SIGNAL
    m_dsp->drawBox(95, 1, 2, 3); // ...  BAR SIGNAL
    m_dsp->drawBox(98, 0, 2, 4); // HIGH BAR SIGNAL
    
    m_dsp->drawLine(0, 7, 128, 7);
    
    //m_dsp->setFont(u8g2_font_t0_11_te);
    //sprintf(dmp, "%.1f", Syncer.get_delta_time() * 0.001f);
    //m_dsp->drawUTF8(0, 60, dmp);
}

void Displayer::draw_bottom()
{
    static char dmp[64];

    m_dsp->setFont(u8g2_font_crox1h_tf); // u8g2_font_Born2bSportyV2_tf, u8g2_font_9x6LED_tf
    
    {
        float time_remaining_perc = (get_time_ms() - g_data.last_forecast_update) * 1.0f / host_hora;
        if (time_remaining_perc > 1.0f) time_remaining_perc = 1.0f;
        if (time_remaining_perc < 0.0f) time_remaining_perc = 0.0f; // somehow, idk

        sprintf(dmp, u8"%.1fºC %.0f%% Chuva: %.0f%%", g_data.last_data[0].temp, g_data.last_data[0].umid * 100.0f, 100.0f * (g_data.rain_prob[0] * (0.5f + 0.5f * time_remaining_perc) + g_data.rain_prob[1] * (0.5f + 0.5f * (1.0f - time_remaining_perc))));
        const auto len = m_dsp->getUTF8Width(dmp);
        m_dsp->drawUTF8(64 - 0.5f * len, 20, dmp);
        m_dsp->drawLine(64 - 0.52f * len, 22, 64 + 0.52f * len, 22);
    }

    const uint8_t limit_y = 24;
    const uint8_t limit_max_y = 63;

    for(size_t p = 0; p < (alg_history_size - 1); ++p) {
        m_dsp->drawLine(
            128.0f - 128.0f * (static_cast<float>(p) * 1.0f / (alg_history_size - 1)), limit_y + ((limit_max_y - limit_y) * 1.0f * (1.0f - g_data.rain_prob[p])),
            128.0f - 128.0f * (static_cast<float>(p + 1) * 1.0f / (alg_history_size - 1)), limit_y + ((limit_max_y - limit_y) * 1.0f * (1.0f - g_data.rain_prob[p + 1]))
        );
    }
}

void Displayer::flip()
{
    if (m_dsp) {
        m_dsp->sendBuffer();
        m_dsp->clearBuffer();
    }
}

Displayer::~Displayer()
{
    destroy();
}

void Displayer::draw()
{
    if (!m_dsp) {
        m_dsp = new U8G2_SSD1306_128X64_NONAME_F_SW_I2C(U8G2_R0, DISP_CLK_PIN, DISP_DATA_PIN, DISP_RST_PIN);
        m_dsp->begin();
        m_dsp->enableUTF8Print();
    }
    
    draw_top();
    draw_bottom();
    flip();
}

void Displayer::destroy()
{
    if (m_dsp) {
        delete m_dsp;
        m_dsp = nullptr;
    }
}