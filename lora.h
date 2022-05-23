#pragma once

#include "heltec.h"
#include "Arduino.h"

#include <mutex>
#include <functional>

#define POWER 20 // [2..20]
#define BAND    ((long)915E6)  //you can set band here directly,e.g. 868E6,915E6
#define SYNC_WORD 0x3F // [0x00 - 0xFF]

namespace LoRaAsync {

  constexpr size_t SIGNATURE_LEN = 4;
  static char THIS_SIGNATURE[SIGNATURE_LEN] = {'L', 'S', 'W', '1'};
  
  struct __pack_dbg_insert {
     char SIGNATURE_OFFSET[4];
  };
  
  struct pack {
     std::vector<char> data; // raw data.
     uint32_t rssi = 0; // typ: [0, 120], common: [25, 100] [dB] (inverse)
     float snr = 0.0f; // typ: [-20, 10] [dB]
  };
  
  struct __storage {
    std::mutex packs_mtx;
    std::function<void(pack&)> func_deal;
    
    bool has_initialized = false;
  };
  
  static __storage __strlora;
  
  char* __cast_begin_p(__pack_dbg_insert*);
  char* __cast_end_p(__pack_dbg_insert*);
  
  bool __lora_receive_confirm(uint64_t);
  void __lora_on_receive(int packsiz);
  
  uint64_t lora_get_mac();
  std::string lora_get_mac_str(); // https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/blob/master/esp32/libraries/ESP32/examples/ChipID/GetChipID/GetChipID.ino
  
  void lora_init(char = 'L', char = 'S', char = 'W', char = '1');
  void lora_stop();
  
  void lora_hook_recv(std::function<void(pack&)>);
  void lora_unhook_recv();
  
  bool lora_send(const char*, size_t);//, uint32_t = 0);
  bool lora_send(std::vector<char>);//, uint32_t = 0);
  
}
