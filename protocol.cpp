#include "protocol.h"
/*
bool send_pack(protocol& pk)
{
  pk.device = LoRaAsync::lora_get_mac();
  return LoRaAsync::lora_send((char*)&pk, sizeof(pk));
}

bool convert_pack(const std::vector<char> dat, protocol_extra& pk)
{
  LoRaAsync::pack raw;
  if (!LoRaAsync::lora_recv(raw, 200)) return false;
  
  if (dat.size() != sizeof(pk.dat)) {
    return false;
  }

  memcpy((void*)&pk.dat, raw.data.data(), raw.data.size());
  if (pk.dat.device == LoRaAsync::lora_get_mac()) return false; // heard itself?
  pk.signal_strength = raw.rssi;
  pk.snr = raw.snr;
  return true;
}*/
