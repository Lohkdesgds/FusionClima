#include "protocol_combo.h"
#define ESP_PLATFORM
#include <DHT.h>
#include <DHT_U.h>
#include <U8g2lib.h>
#include "heltec.h"
#include <string>
#include "references.h"

/* -------------------- External references -------------------- */
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C m_dsp;
extern const uint8_t pin_LED;

/* -------------------- Local stuff -------------------- */
constexpr uint8_t pin_DHT22 = 32;
DHT dht(pin_DHT22, DHT22);

void as_monitor(void* ___nothin)
{
  PRTLN("Starting monitoring...");
  
  dht.begin();
  Comm::PROT_LORA_DEFAULTS();
  
  float temp, hum;

  while(1){
    DISPLAYBLOCK(
      m_dsp.setFont(u8g2_font_t0_11_te);
      m_dsp.drawStr(0, 16, AUTOF("Temp: %.1f Hum: %.1f%%", temp, hum));
      m_dsp.drawStr(0, 32, AUTOF("Self: #%llX", Comm::lora_mac));
    );
    
    delay(5000);

    temp = dht.readTemperature();
    hum = dht.readHumidity();
    
    //PRTLN("Temperature: %.1f; Humidity: %.1f%%", temp, hum);
    
    SLED(HIGH);
    PRTLN("Broadcasting...");
    LoRa.beginPacket();    
    Comm::generic_format ev;
    ev.ident = Comm::lora_mac;
    ev.temp_x10 = static_cast<uint16_t>(10.0f * temp);
    ev.hum_x10 = static_cast<uint16_t>(10.0f * hum);
    LoRa.write((uint8_t*)&ev, sizeof(ev));
    LoRa.endPacket();
    PRTLN("End of broadcast.");
    delay(10);
    SLED(LOW);
  }
}