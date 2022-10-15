#define ESP_PLATFORM
#include <U8g2lib.h>
#include "heltec.h"
#include "references.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C m_dsp(U8G2_R0, 16, 15, 4);//(U8G2_R0, 15, 4, 16);

void setup() {
  pinMode(pin_LED, OUTPUT);
  SLED(HIGH);

  Serial.begin(115200);  
  pinMode(pin_SWITCH, INPUT_PULLUP);
  m_dsp.setBusClock(800000);
  
  if (!m_dsp.begin()) errcoderst(2);

  if (!LoRa.begin(915E6, true)) errcoderst(3);

  delay(10);
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
