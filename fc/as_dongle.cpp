#include "as_dongle.h"

/* -------------------- External references -------------------- */
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C m_dsp;
extern const uint8_t pin_LED;
extern uint32_t get_lowest_clock();

/* -------------------- Local stuff -------------------- */
constexpr float history_weight_f = 5.0f;
AsDongle* _dgl_inmem = nullptr;

void IRAM_ATTR on_dglLoRaRecv(int size)
{
    if (sizeof(Comm::generic_format) != size) return;
    
    Comm::generic_format ev;
    LoRa.readBytes((uint8_t*)&ev, sizeof(ev));
    xQueueSendToBackFromISR(_dgl_inmem->dongle_que, &ev, NULL);
}

// Smooths stuff
void thr_dongle0(void* arg)
{
    AsDongle& dgldat = *(AsDongle*)arg;
    dgldat.smooth_rssi = static_cast<float>(LoRa.packetRssi());
    dgldat.smooth_snr = LoRa.packetSnr();

    uint8_t flippr = 0;

    while(1) {
        delay(48);
        if ((flippr = (flippr + 1) % 4) == 1) {
            std::lock_guard<std::mutex> l(dgldat.dgl_mtx);
            dgldat.real_rssi = static_cast<float>(LoRa.packetRssi());
            dgldat.real_snr = LoRa.packetSnr();
        }
        dgldat.smooth_rssi = (dgldat.smooth_rssi * history_weight_f + dgldat.real_rssi) / (1.0f + history_weight_f);
        dgldat.smooth_snr = (dgldat.smooth_snr * history_weight_f + dgldat.real_snr) / (1.0f + history_weight_f);
    }
}

// keeps Serial fed.
void thr_dongle1(void* arg)
{
    AsDongle& dgldat = *(AsDongle*)arg;
    while(1) {
        delay(200);
        {
            std::lock_guard<std::mutex> l(dgldat.dgl_mtx);
            dgldat.dgl_ev.pck_id = dgldat.packet_counter;
            dgldat.dgl_ev.rssi_x10 = static_cast<int16_t>(10.0f * dgldat.real_rssi);
            dgldat.dgl_ev.snr_x10 = static_cast<int16_t>(10.0f * dgldat.real_snr);
        }
        Serial.write((uint8_t*)&dgldat.dgl_ev, sizeof(dgldat.dgl_ev));
    }
}

// check for serial tweet request.
void thr_dongle2(void* arg)
{
    AsDongle& dgldat = *(AsDongle*)arg;
    while(1) {
        if (Serial.available()) {
            std::string cmd;
            {
                char ch;
                while((ch = Serial.read()) != '\n') cmd += ch;
            }
            
            if (cmd == "request") {
                dongle_tweet();
            }
            
        }
        delay(100);
    }
}

void as_dongle(void* ___nothin)
{
    _dgl_inmem = new AsDongle(); // hack to avoid stack use
    AsDongle& dgldat = *_dgl_inmem; // keep as ref, but alloc mem only.
    
    LoRa.onReceive(on_dglLoRaRecv);
    Comm::PROT_LORA_DEFAULTS();
    LoRa.receive();

    strcpy(dgldat.dgl_ev.trigg, Comm::protID);

    xTaskCreate(thr_dongle0, "dglSMOOTH", 2048, (void*)&dgldat, 4, nullptr);
    xTaskCreate(thr_dongle1, "dglSERGAL", 2048, (void*)&dgldat, 5, nullptr);
    xTaskCreate(thr_dongle2, "dglSERCMD", 2048, (void*)&dgldat, 5, nullptr);
    
    dongle_tweet();

    m_dsp.setFont(u8g2_font_t0_11_te);

    while(1){
        delay(10);
        LoRa.receive();
        
        Comm::generic_format ev;
        
        if (xQueueReceive(dgldat.dongle_que, &ev, 0) != pdFALSE) {
            if (ev.reqtype == static_cast<uint8_t>(Comm::request_id::TEMPHUMUPDATE)) {
                std::lock_guard<std::mutex> l(dgldat.dgl_mtx);
                memcpy(&dgldat.dgl_ev.data, &ev, sizeof(ev));
                dgldat.last_temp = 0.1f * dgldat.dgl_ev.data.data.th.temp_x10;
                dgldat.last_hum = 0.1f * dgldat.dgl_ev.data.data.th.hum_x10;
                dgldat.last_mac = dgldat.dgl_ev.data.ident;
                ++dgldat.packet_counter;
            }
        }

        DISPLAYBLOCK(
            m_dsp.drawStr(0, 12, AUTOF("RSSI: %.2f", dgldat.smooth_rssi));
            m_dsp.drawStr(0, 24, AUTOF("SNR: %.2f", dgldat.smooth_snr));
            m_dsp.drawUTF8(0, 36, AUTOF("Temp: %.1f ºC", dgldat.last_temp));
            m_dsp.drawStr(0, 48, AUTOF("Hum: %.1f %%", dgldat.last_hum));
            m_dsp.drawStr(0, 60, AUTOF("#%llX", dgldat.last_mac));
        );
        
    }
}

// ask for information nearby
void dongle_tweet()
{
    SLED(HIGH);
    LoRa.beginPacket();    
    Comm::generic_format ev;
    ev.ident = Comm::lora_mac;
    ev.reqtype = static_cast<uint8_t>(Comm::request_id::REQUESTDATA);
    LoRa.write((uint8_t*)&ev, sizeof(ev));
    LoRa.endPacket();
    delay(10);
    SLED(LOW);
    LoRa.receive();
}