#include "sensor.h"
#include "display.h"
#include "protocol.h"
#include "shared.h"
#include "button.h"
#include "battery.h"

// useful: https://resource.heltec.cn/download/WiFi_LoRa_32/WIFI_LoRa_32_V2.pdf

//uint32_t LORA_WITH_SENSOR_LICENSE[4] = {0x3CFF7D63,0xAC4833CE,0xA7C0C3F4,0x5C46868C};
//uint32_t LORA_LISTENING_NO_READING[4] = {0x1701DDF6,0x0A7003DB,0x91DE8C37,0x6459DE6F};

constexpr int32_t ESP_MODE_PIN = 17;
constexpr int32_t ESP_LED = 25;

enum class ESP_MODE { SENSOR, LISTENER };

bool had_issue = false;
bool was_awake = false;
bool next_shutdown = false;
decltype(millis()) last_batt_upd = 0;

Displayer* disp = nullptr;
ESP_MODE work_as = ESP_MODE::SENSOR;
Sensor sens([](e_sensor_errors issue){had_issue = true;});
std::thread async_stuff; // valid for receiver

void async_stuff_do();
void update_bat();
void shutdown_do();
void change_mode();

void setup()
{
  //Serial.begin(115200);
    
  if (!was_awake || disp) {
    pinMode(ESP_MODE_PIN, INPUT_PULLUP);
    pinMode(ESP_LED, OUTPUT);
  
    digitalWrite(ESP_LED, HIGH);  
    delay(500);
    digitalWrite(ESP_LED, LOW);
    
    work_as = digitalRead(ESP_MODE_PIN) != HIGH ? ESP_MODE::SENSOR : ESP_MODE::LISTENER;
  
    disp = new Displayer();
    disp->set_mode(work_as == ESP_MODE::SENSOR ? e_display_mode::SENDER_DEFAULT : e_display_mode::RECEIVER_TEMP);
    
    if (work_as == ESP_MODE::LISTENER) {
      setup_battery();
      on_button_single(change_mode);
      on_button_double(shutdown_do);
      setup_button();
      async_stuff = std::thread(async_stuff_do);
    }
      
    //Serial.print("Started device ID ");
    //Serial.println(LoRaAsync::lora_get_mac_str().c_str());
    
    LoRaAsync::lora_init();
  }
  else {
    digitalWrite(ESP_LED, HIGH);  
    delay(200);
    digitalWrite(ESP_LED, LOW);
    disp->sleeping(false);
    LoRaAsync::lora_init();
  }
}

void loop()
{  
  switch(work_as) {
  case ESP_MODE::SENSOR:
  {
    const int time_send = 5;

    protocol pkg;
    pkg.temp = sens.get_temperature();
    pkg.humd = sens.get_humidity();
    pkg.temp_d = sens.get_temperature_last_delta();
    pkg.humd_d = sens.get_humidity_last_delta();
    
    disp->sender(time_send * 1000, pkg.temp, pkg.humd, POWER);
    delay(time_send * 1000);
    send_pack(pkg);
  }
    break;
  case ESP_MODE::LISTENER:
  {
    protocol_extra pk;
    if (try_get_pack(pk)) {

      if (pk.err == e_protocol_err::NONE) {
        disp->receiver(pk.signal_strength, pk.snr, pk.dat.temp, pk.dat.humd);
      }
    }
    delay(250);
  }
    break;
  }
}

void async_stuff_do()
{
  while(1) {
    update_bat();
    
    if (next_shutdown) {
      next_shutdown = false;
      if (disp) {
        disp->sleeping(true);
      }
      prepare_button_deep_sleep();
      esp_deep_sleep_start();
    }
    delay(250);
  }
}

void update_bat()
{
  if (millis() - last_batt_upd > 2000) {
    float _f = read_battery_perc(true);
    last_batt_upd = millis();

    disp->set_battery(_f, get_battery_charging());
  }
}

void shutdown_do()
{
  next_shutdown = true;
}

void change_mode()
{
  if (disp) {
    disp->next_mode();
    disp->sleeping(false);
  }
}
