#include "display.h"
#include "sensor.h"

// useful: https://resource.heltec.cn/download/WiFi_LoRa_32/WIFI_LoRa_32_V2.pdf

constexpr int32_t ESP_MODE_PIN = 17;
constexpr int32_t ESP_LED = 25;
enum class ESP_MODE { SENSOR, LISTENER };

bool had_issue = false;
Display* disp = nullptr;
ESP_MODE work_as = ESP_MODE::SENSOR;
Sensor sens([](e_sensor_errors issue){had_issue = true;});

void setup()
{
  pinMode(ESP_MODE_PIN, INPUT_PULLUP);
  pinMode(ESP_LED, OUTPUT);

  digitalWrite(ESP_LED, HIGH);  
  delay(500);
  digitalWrite(ESP_LED, LOW);  
  
  work_as = digitalRead(ESP_MODE_PIN) != HIGH ? ESP_MODE::SENSOR : ESP_MODE::LISTENER;

  disp = new Display();

  switch(work_as) {
  case ESP_MODE::SENSOR:
    disp->set_temp_custom_text("SENSOR MODE");
    delay(2000);
    break;
  case ESP_MODE::LISTENER:
    disp->set_temp_custom_text("LISTENER MODE");
    break;
  }
}

void loop()
{
  delay(980);
  digitalWrite(ESP_LED, HIGH);
  delay(20);
  digitalWrite(ESP_LED, LOW);

  char dummy[48];
  sprintf(dummy, "%c %.2f|%.3f C ", (had_issue ? '!' : 'K'), sens.get_temperature(), sens.get_temperature_last_delta());

  disp->set_temp_custom_text(dummy);
  
}
