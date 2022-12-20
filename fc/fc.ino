#define ESP_PLATFORM
#include <U8g2lib.h>
#include "heltec.h"
#include "esp_pm.h"
#include "references.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C m_dsp(U8G2_R0, 16, 15, 4);//(U8G2_R0, 15, 4, 16);

uint32_t get_lowest_clock() {    
    switch(getXtalFrequencyMhz()) {
    case RTC_XTAL_FREQ_40M:
        return 10;
    case RTC_XTAL_FREQ_26M:
        return 13;
    case RTC_XTAL_FREQ_24M:
        return 12;
    default:
        return 80;
    }
}

void setup() {  
  m_dsp.setBusClock(800000);
  
  pinMode(pin_LED, OUTPUT);
  SLED(HIGH);

  Serial.begin(115200);  
  
  Serial.printf("[Debug] Min clock: %u\n", get_lowest_clock());
  
  pinMode(pin_SWITCH, INPUT_PULLUP);    
  
  if (!m_dsp.begin()) errcoderst(2);

  if (!LoRa.begin(915E6, true)) errcoderst(3);
  
  delay(100);
  SLED(LOW);
}

void loop() {  
  if (IS_DONGLE()) as_dongle(nullptr);//xTaskCreate(as_dongle, "mainTask", 6144, nullptr, 4, nullptr);
  else as_monitor(nullptr);//xTaskCreate(as_monitor, "mainTask", 6144, nullptr, 4, nullptr);

  vTaskDelete(NULL);
}

void errcoderst(uint8_t cod)
{
  SLED(LOW);
  delay(1000);
  for(uint8_t a = 0; a < cod; ++a) {  
    SLED(HIGH);
    delay(50);
    SLED(LOW);
    delay(450);
  }
  delay(5000);
  ESP.restart();
}
