#pragma once

#ifdef ESP_PLATFORM 
#include "heltec.h"
#else
#include <stdint.h>
#endif

namespace Comm {
  extern const char protID[];
  
  enum class request_id : uint8_t {TEMPHUMUPDATE = 1, REQUESTDATA = 2};

  struct generic_format { // received/sent via LORA
    uint64_t ident = 0; // mac
    uint8_t reqtype = 0; // for later use, if needed
    union raw {
        struct { uint16_t temp_x10 = 0, hum_x10 = 0; } th; // temp and hum
    } data{};
  };
  

  struct usb_format_raw {
    char trigg[16]{0}; // should have protID
    int16_t rssi_x10 = 0, snr_x10 = 0;
    uint8_t pck_id = 0; // used just to avoid adding same data to graph
    generic_format data;
  };

#ifdef ESP_PLATFORM 
  extern const uint64_t lora_mac;
  extern const uint8_t lora_syncword;

  inline void PROT_LORA_DEFAULTS() { LoRa.setTxPower(17, PA_OUTPUT_PA_BOOST_PIN); LoRa.setSpreadingFactor(12); /*LoRa.setSignalBandwidth(62.5E3);*/ LoRa.setPreambleLength(32); LoRa.setSyncWord(Comm::lora_syncword); }
#endif

}