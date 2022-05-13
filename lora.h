#pragma once

#include "heltec.h"
#include "Arduino.h"
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>

#define BAND    ((long)915E6)  //you can set band here directly,e.g. 868E6,915E6
#define SYNC_WORD 0x3F // [0x00 - 0xFF]

namespace LoRaAsync {
  
  struct package {
    String str;
    int signal_strength = 0; // typ: [0, -120] [dB]
    float snr = 0; // typ: [-20, 10] [dB]

    package() = default;
    ~package();
    package(package&&);
    package(const package&) = delete;
    void operator=(package&&);
    void operator=(const package&) = delete;

    void write(const char* dat, size_t ln);
    size_t size() const;
    const char* data() const;
  };

  struct __global_data {
    std::mutex mtx;
    package last_pack;
    bool has_pack = false;
    bool keep_recv_b = false;
  };

  static __global_data _g_lora;
  

  uint64_t lora_get_mac();
  std::string lora_get_mac_str(); // https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/blob/master/esp32/libraries/ESP32/examples/ChipID/GetChipID/GetChipID.ino

  void _lora_loop();

  void lora_init();
  void lora_stop();
  
  bool lora_send(char*, size_t);
  bool lora_pop_recv(package&);

  size_t lora_recv_length();
  
}
