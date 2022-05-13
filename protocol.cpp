#include "protocol.h"

bool send_pack(protocol& pk)
{
  pk.device = LoRaAsync::lora_get_mac();
  memcpy(&pk.signature, &SIGNATURE, sizeof(SIGNATURE));  
  return LoRaAsync::lora_send((char*)&pk, sizeof(pk));
}

bool try_get_pack(protocol_extra& pk)
{
  LoRaAsync::package raw;

  for(size_t _c = 0; !LoRaAsync::lora_pop_recv(raw); ++_c){
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (_c >= 10) return false;
  }
  
  if (raw.size() != sizeof(pk.dat)) {
    pk.err = e_protocol_err::PACKAGE_SIZE_MISMATCH;
    return false;
  }

  memcpy((void*)&pk.dat, raw.data(), raw.size());
  if (memcmp(&SIGNATURE, &pk.dat.signature, sizeof(SIGNATURE)) != 0) {
    pk.err = e_protocol_err::SIGNATURE_FAIL;
    return false;
  }
  
  pk.signal_strength = raw.signal_strength;
  pk.snr = raw.snr;
  pk.err = e_protocol_err::NONE;  
  return true;
}
