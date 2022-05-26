#include "display.h"

DisplayWork::Displayer display;

namespace DisplayWork {
    
    void Displayer::draw_top()
    {
        static char dmp[64];
        m_dsp->drawXBM(0, 0, 128, 6, u8g_topbar2);
        
        //m_data
        /*
        unsigned long long last_received = 0; // expects get_time_ms()
        float cpu_usage_perc = 0.0f; // expect [0.0..1.0]
        float ram_free_perc = 0.0f; // expect [1.0..0.0]
        uint16_t signals_received = 0; // expect [0..99]
        uint16_t signal_rssi = 0; // expects [0..100], inverse of rssi
        float batt_perc = 0.0f; // expects [0.0..1.0]
        */
        m_dsp->setFont(u8g2_font_tiny_simon_tr); 
        
        // CPU USAGE
        { const float _cpu = 99.9f * get_cpu_usage(); if (_cpu >= 10.0f) {sprintf(dmp, "%04.1f%%", _cpu);} else {sprintf(dmp, "%04.2f%%", _cpu);}}
        m_dsp->drawUTF8(0, 5, dmp); // CPU USAGE
        
        // RAM USAGE
        { const float _ram = 99.9f * get_ram_usage(); if (_ram >= 10.0f) {sprintf(dmp, "%04.1f%%", _ram);} else {sprintf(dmp, "%04.2f%%", _ram);}}
        //sprintf(dmp, "%04.1f%%", 99.9f * get_ram_usage());  
        m_dsp->drawUTF8(24, 5, dmp); 
        
        m_dsp->drawUTF8(43, 5, "88"); // DEVICES ECHOING (TRANSMITTERS RECEIVED)
        m_dsp->drawUTF8(56, 5, "88"); // TIME SINCE LAST RECV (minutes)
        
        // LAST BEST SIGNAL (-dB)
        //m_dsp->drawUTF8(68, 5, "188dB");
        { const int32_t _rssi = LR.currentRssi(); if (_rssi > 0) {sprintf(dmp, "00dB");} else if (_rssi > -199) {sprintf(dmp, "%02idB", -_rssi);} else {sprintf(dmp, "-199dB");} }
        m_dsp->drawUTF8(68, 5, dmp); // LAST BEST SIGNAL (-dB)
        
        m_dsp->drawUTF8(113, 5, "188%%"); // BATTERY PERC [0..100]  
        m_dsp->drawLine(109, 2, 105, 2);  // BATTERY BAR
        m_dsp->drawBox(89, 3, 2, 1); // LOW  BAR SIGNAL
        m_dsp->drawBox(92, 2, 2, 2); // ...  BAR SIGNAL
        m_dsp->drawBox(95, 1, 2, 3); // ...  BAR SIGNAL
        m_dsp->drawBox(98, 0, 2, 4); // HIGH BAR SIGNAL
        
        m_dsp->drawLine(0, 7, 128, 7);  
    }

    void Displayer::draw_bottom()
    {
    }

    void Displayer::draw_debug()
    {
      
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

    display_data& Displayer::get()
    {
        return m_data;
    }

    void Displayer::draw()
    {
        if (!m_dsp) {
            m_dsp = new U8G2_SSD1306_128X64_NONAME_F_SW_I2C(U8G2_R0, DISP_CLK_PIN, DISP_DATA_PIN, DISP_RST_PIN);
            m_dsp->begin();
            m_dsp->enableUTF8Print();
        }
        if (m_data.debug_literal) {
            draw_top();      
        }
        else {
            draw_top();
            draw_bottom();
        }
        flip();
    }

    void Displayer::destroy()
    {
        if (m_dsp) {
            delete m_dsp;
            m_dsp = nullptr;
        }
    }
}