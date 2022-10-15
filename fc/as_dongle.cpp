#include "protocol_combo.h"
#define ESP_PLATFORM
#include <U8g2lib.h>
#include "heltec.h"
#include <string.h>
#include "protocol_combo.h"
#include "references.h"
#include <mutex>

/* -------------------- External references -------------------- */
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C m_dsp;
extern const uint8_t pin_LED;

/* -------------------- Local stuff -------------------- */
constexpr float history_weight_f = 5.0f;
QueueHandle_t dongle_que = nullptr;
volatile float last_temp = 0.0f;
volatile float last_hum = 0.0f;
uint64_t last_mac = 0;
volatile float smooth_rssi = 0.0f, smooth_snr = 0.0f, real_rssi = 0.0f, real_snr = 0.0f;
volatile uint8_t packet_counter = 0;
Comm::usb_format_raw dgl_ev{};
std::mutex dgl_mtx;

void IRAM_ATTR on_LoraReceive(int size)
{
  if (sizeof(Comm::generic_format) != size) return;
  
  Comm::generic_format ev;
  LoRa.readBytes((uint8_t*)&ev, sizeof(ev));
  xQueueSendToBackFromISR(dongle_que, &ev, NULL);
}

// Smooths stuff
void thr_dongle0(void* __nii)
{
  smooth_rssi = static_cast<float>(LoRa.packetRssi());
  smooth_snr = LoRa.packetSnr();

  uint8_t flippr = 0;

  while(1) {
    delay(48);
    if ((flippr = (flippr + 1) % 4) == 1) {
      std::lock_guard<std::mutex> l(dgl_mtx);
      real_rssi = static_cast<float>(LoRa.packetRssi());
      real_snr = LoRa.packetSnr();
    }
    smooth_rssi = (smooth_rssi * history_weight_f + real_rssi) / (1.0f + history_weight_f);
    smooth_snr = (smooth_snr * history_weight_f + real_snr) / (1.0f + history_weight_f);
  }
}

// keeps Serial fed.
void thr_dongle1(void* __nii)
{
  while(1) {
    delay(200);
    {
      std::lock_guard<std::mutex> l(dgl_mtx);
      dgl_ev.pck_id = packet_counter;
      dgl_ev.rssi_x10 = static_cast<int16_t>(10.0f * real_rssi);
      dgl_ev.snr_x10 = static_cast<int16_t>(10.0f *real_snr);
    }
    Serial.write((uint8_t*)&dgl_ev, sizeof(dgl_ev));
  }
}

void as_dongle(void* ___nothin)
{
  dongle_que = xQueueCreate(4, sizeof(Comm::generic_format));
  LoRa.onReceive(on_LoraReceive);
  Comm::PROT_LORA_DEFAULTS();
  LoRa.receive();

  int last_was_new = 0;
  strcpy(dgl_ev.trigg, Comm::protID);

  xTaskCreate(thr_dongle0, "dglSMOOTH", 2048, nullptr, 4, nullptr);
  xTaskCreate(thr_dongle1, "dglSERGAL", 2048, nullptr, 5, nullptr);

  m_dsp.setFont(u8g2_font_t0_11_te);

  while(1){
    delay(10);
    LoRa.receive();
    
    if (xQueueReceive(dongle_que, &dgl_ev.data, 0) != pdFALSE) {      
      std::lock_guard<std::mutex> l(dgl_mtx);
      last_temp = 0.1f * dgl_ev.data.temp_x10;
      last_hum = 0.1f * dgl_ev.data.hum_x10;
      last_mac = dgl_ev.data.ident;
      ++packet_counter;
      last_was_new = 100;      
      //PRTLN("Has new info: %.1f %.1f %llu", last_temp, last_hum, last_mac);
      //delay(1000);
    }

    DISPLAYBLOCK(
      m_dsp.drawStr(0, 12, AUTOF("RSSI: %.2f", smooth_rssi));
      m_dsp.drawStr(0, 24, AUTOF("SNR: %.2f", smooth_snr));
      m_dsp.drawUTF8(0, 36, AUTOF("Temp: %.1f ºC", last_temp));
      m_dsp.drawStr(0, 48, AUTOF("Hum: %.1f %%", last_hum));
      if (last_was_new) {
        m_dsp.drawStr(0, 60, AUTOF("V%llX", last_mac));
        --last_was_new;
      }
      else m_dsp.drawStr(0, 60, AUTOF("#%llX", last_mac));
    );
    
  }
}