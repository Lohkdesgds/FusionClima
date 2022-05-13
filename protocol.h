#pragma once

#include <Arduino.h>
#include "lora.h"

enum class e_protocol_err{NONE, PACKAGE_SIZE_MISMATCH, SIGNATURE_FAIL};

const char SIGNATURE[] = {'L', 'S', 'W', 'U', '0', '0'};

struct protocol {
  char signature[sizeof(SIGNATURE)];
  uint64_t device = 0;
  float temp = 0.0f, temp_d = 0.0f;
  float humd = 0.0f, humd_d = 0.0f;
};

struct protocol_extra {
  protocol dat;
  e_protocol_err err = e_protocol_err::NONE;
  int signal_strength = 0; // typ: [0, -120] [dB]
  float snr = 0; // typ: [-20, 10] [dB]
};

bool send_pack(protocol&);
bool try_get_pack(protocol_extra&);
