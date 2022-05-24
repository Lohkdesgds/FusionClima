#pragma once

#include <Arduino.h>
#include "lora.h"

struct protocol {
  uint64_t device = 0;
  float temp = 0.0f, temp_d = 0.0f;
  float humd = 0.0f, humd_d = 0.0f;
};

struct protocol_extra {
  protocol dat;
  int signal_strength = 0; // typ: [0, -120] [dB]
  float snr = 0; // typ: [-20, 10] [dB]
};

bool send_pack(protocol&);
bool try_get_pack(protocol_extra&);
