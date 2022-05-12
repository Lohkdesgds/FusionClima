#include "sensor.h"
#include "display.h"
#include "protocol.h"

// useful: https://resource.heltec.cn/download/WiFi_LoRa_32/WIFI_LoRa_32_V2.pdf

//uint32_t LORA_WITH_SENSOR_LICENSE[4] = {0x3CFF7D63,0xAC4833CE,0xA7C0C3F4,0x5C46868C};
//uint32_t LORA_LISTENING_NO_READING[4] = {0x1701DDF6,0x0A7003DB,0x91DE8C37,0x6459DE6F};

const std::string VERSION = "V1.0_2205121613";

constexpr int32_t ESP_MODE_PIN = 17;
constexpr int32_t ESP_LED = 25;
enum class ESP_MODE { SENSOR, LISTENER };

bool had_issue = false;
Displayer* disp = nullptr;
ESP_MODE work_as = ESP_MODE::SENSOR;
Sensor sens([](e_sensor_errors issue){had_issue = true;});

void setup()
{
  Serial.begin(115200);
  pinMode(ESP_MODE_PIN, INPUT_PULLUP);
  pinMode(ESP_LED, OUTPUT);

  digitalWrite(ESP_LED, HIGH);  
  delay(500);
  digitalWrite(ESP_LED, LOW);  
  
  work_as = digitalRead(ESP_MODE_PIN) != HIGH ? ESP_MODE::SENSOR : ESP_MODE::LISTENER;

  disp = new Displayer();
  
  Serial.print("Started device ID");
  Serial.println(LoRaAsync::lora_get_mac_str().c_str());

  switch(work_as) {
  case ESP_MODE::SENSOR:
    disp->set_temp_custom_text(("SND|" + VERSION).c_str(), 0);
    disp->set_temp_custom_text("This device ID:", 1);
    disp->set_temp_custom_text(LoRaAsync::lora_get_mac_str(), 2);
    delay(5000);
    disp->set_temp_custom_text("", 2);
    break;
  case ESP_MODE::LISTENER:
    disp->set_temp_custom_text(("LSN|" + VERSION).c_str(), 0);
    disp->set_temp_custom_text("This device ID:", 1);
    disp->set_temp_custom_text(LoRaAsync::lora_get_mac_str(), 2);
    delay(5000);
    disp->set_temp_custom_text("", 2);
    break;
  }
  
  LoRaAsync::lora_init();
}

void loop()
{
  switch(work_as) {
  case ESP_MODE::SENSOR:
  {
    disp->set_temp_custom_text("", 1);
    disp->set_temp_custom_text("", 2);
    
    for(int a = 2; a != 0; --a) {
      char buf[24];
      sprintf(buf, "Next send in %d...", a);
      disp->set_temp_custom_text(buf, 1);
      disp->set_temp_custom_text("", 2);
      delay(1000);
    }

    disp->set_temp_custom_text("Working on pack...", 1);
    
    protocol pkg;
    pkg.temp = sens.get_temperature();
    pkg.humd = sens.get_humidity();
    pkg.temp_d = sens.get_temperature_last_delta();
    pkg.humd_d = sens.get_humidity_last_delta();

    {
      char buf[30];
      sprintf(buf, "%.1f|%.1fC %.1f|%.1f%c", pkg.temp, pkg.temp_d, pkg.humd, pkg.humd_d, '%');
      disp->set_temp_custom_text(buf, 3);
    }
    
    disp->set_temp_custom_text("Sending...", 1);

    if (send_pack(pkg)) {
      disp->set_temp_custom_text("Success! (emit)", 2);
      delay(1000);
    }
    else {
      disp->set_temp_custom_text("Failed! (emit)", 2);
      delay(1000);
    }
  }
    break;
  case ESP_MODE::LISTENER:
  {
    disp->set_temp_custom_text("Waiting recv...", 1);

    protocol_extra pk;
    while (!try_get_pack(pk)) {
      char buf[24];
      sprintf(buf, "<buf: %zu, err: %d>", LoRaAsync::lora_recv_length(), (int)pk.err);
      disp->set_temp_custom_text(buf, 2);
    }
    disp->set_temp_custom_text("", 2);

    if (pk.err == e_protocol_err::NONE) {
      disp->set_temp_custom_text("Success!", 1);
      {
        char buf[30];
        sprintf(buf, "%.1f|%.1fC %.1f|%.1f%c", pk.dat.temp, pk.dat.temp_d, pk.dat.humd, pk.dat.humd_d, '%');
        disp->set_temp_custom_text(buf, 3);
      }
      disp->set_temp_custom_text("", 2);
      delay(1000);
    }
    else {
      disp->set_temp_custom_text("...", 1);
      char dummy[48];
      sprintf(dummy, "[@%ld] ERROR: %d", millis(), (int)pk.err);
      disp->set_temp_custom_text(dummy, 2);
    }
  }
    break;
  }
  
  /*delay(980);
  digitalWrite(ESP_LED, HIGH);
  delay(20);
  digitalWrite(ESP_LED, LOW);

  char dummy[48];
  
  sprintf(dummy, "%c %.2f|%.3f C ", (had_issue ? '!' : 'K'), sens.get_temperature(), sens.get_temperature_last_delta());
  disp->set_temp_custom_text(dummy, 0);
  
  sprintf(dummy, "%.2f|%.3f H [%.2f] ", sens.get_humidity(), sens.get_humidity_last_delta(), sens.get_heatindex());
  disp->set_temp_custom_text(dummy, 1);*/
  
}
