#include "as_monitor.h"

/* -------------------- External references -------------------- */
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C m_dsp;
extern const uint8_t pin_LED;
extern uint32_t get_lowest_clock();

/* -------------------- Local stuff -------------------- */
AsMonitor* _mnt_inmem = nullptr;
float _l_mnt_temp = 0, _l_mnt_hum = 0;
volatile uint32_t _l_mnt_times_loop_temp = mnt_loop_time_ds;
volatile uint32_t _l_mnt_real_time_temp = mnt_update_time_ds;

void /*IRAM_ATTR*/ on_mntLoRaRecv(int size)
{
    if (sizeof(Comm::generic_format) != size) return;
    
    Comm::generic_format ev;
    LoRa.readBytes((uint8_t*)&ev, sizeof(ev));
    
    if (ev.reqtype == static_cast<uint8_t>(Comm::request_id::REQUESTDATA)) {
        _mnt_inmem->last_tweeter = ev.ident;
        ++_mnt_inmem->tweet_count;
        _mnt_inmem->should_tweet_soon = true;
    }
}

// display update
void thr_monitor0(void* arg)
{
    AsMonitor& mntdat = *(AsMonitor*)arg;
    while(1) {
        if (_l_mnt_real_time_temp) --_l_mnt_real_time_temp;
        if (_l_mnt_times_loop_temp) --_l_mnt_times_loop_temp;
        delay(100);
    }
}

void as_monitor(void* ___nothin)
{
    PRTLN("Starting monitoring...");
  
    _mnt_inmem = new AsMonitor(); // hack to avoid stack use
    AsMonitor& mntdat = *_mnt_inmem; // keep as ref, but alloc mem only.
  
    mntdat.dht.begin();
    //LoRa.onReceive(on_mntLoRaRecv);
    Comm::PROT_LORA_DEFAULTS();
    LoRa.receive();
    
    xTaskCreate(thr_monitor0, "mntDISP", 2048, (void*)&mntdat, 4, nullptr);
            

    setCpuFrequencyMhz(get_lowest_clock());
    while(1){
        DISPLAYBLOCK(
            m_dsp.setFont(u8g2_font_t0_11_te);
            m_dsp.drawStr(0, 12, AUTOF("Temp: %.1f Hum: %.1f%%", _l_mnt_temp, _l_mnt_hum));
            m_dsp.drawStr(0, 24, AUTOF("SelfID: #%llX", Comm::lora_mac));
            m_dsp.drawStr(0, 36, AUTOF("GotTID: #%llX", mntdat.last_tweeter));
            m_dsp.drawStr(0, 48, AUTOF("Tweets: #%llX", mntdat.tweet_count));
            m_dsp.drawStr(0, 60, AUTOF("READ|BC: %.1f|%.1f", _l_mnt_real_time_temp * 0.1f, _l_mnt_times_loop_temp * 0.1f));
        );
                
        if (_l_mnt_real_time_temp == 0) {
            _l_mnt_real_time_temp = mnt_update_time_ds;
            setCpuFrequencyMhz(160);
            _l_mnt_temp = mntdat.dht.readTemperature();
            _l_mnt_hum = mntdat.dht.readHumidity();
            int packetSize = LoRa.parsePacket();
            if (packetSize) on_mntLoRaRecv(packetSize);
            while(LoRa.available()) LoRa.read();
            setCpuFrequencyMhz(get_lowest_clock());          
        }

        if (_l_mnt_times_loop_temp == 0 || exchange(mntdat.should_tweet_soon, false)) {
            mntdat.should_tweet_soon = false;
            _l_mnt_times_loop_temp = mnt_loop_time_ds;
            SLED(HIGH);
            PRTLN("Broadcasting...");
            setCpuFrequencyMhz(160);
            LoRa.beginPacket();        
            Comm::generic_format ev;
            ev.ident = Comm::lora_mac;
            ev.reqtype = static_cast<uint8_t>(Comm::request_id::TEMPHUMUPDATE);
            ev.data.th.temp_x10 = static_cast<uint16_t>(10.0f * _l_mnt_temp);
            ev.data.th.hum_x10 = static_cast<uint16_t>(10.0f * _l_mnt_hum);
            LoRa.write((uint8_t*)&ev, sizeof(ev));
            LoRa.endPacket();
            LoRa.receive();
            PRTLN("End of broadcast.");
            delay(10);
            SLED(LOW);
            setCpuFrequencyMhz(get_lowest_clock());
        }
    }
}

AsMonitor::AsMonitor() 
    : dht(pin_DHT22, DHT22) // Arduino IDE cannot do this on a header somehow.
{
}