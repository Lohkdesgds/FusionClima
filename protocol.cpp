#include "protocol.h"

bool send_pack(protocol& pk)
{
  pk.device = LoRaAsync::lora_get_mac();
  memcpy(&pk.signature, &SIGNATURE, sizeof(SIGNATURE));
  
  /*Serial.printf("===========\nSending pack:\nDEVICE: %llu", pk.device);
  Serial.print("\nSIGNATURE: ");
  Serial.print(pk.signature);
  Serial.print("\nDATA: ");
  Serial.printf("%.2f, %.2f\n", pk.temp, pk.humd);*/
  
  return LoRaAsync::lora_send((char*)&pk, sizeof(pk));
}

bool try_get_pack(protocol_extra& pk)
{
  LoRaAsync::package raw;
  
  //Serial.print("===========\nWaiting pack...");

  for(size_t _c = 0; !LoRaAsync::lora_pop_recv(raw); ++_c){
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (_c >= 10) return false;
  }
  
  if (raw.size() != sizeof(pk.dat)) {
    //Serial.print("\nERR: Wrong pack size.\n");
    pk.err = e_protocol_err::PACKAGE_SIZE_MISMATCH;
    return false;
  }

  memcpy((void*)&pk.dat, raw.data(), raw.size());
  if (memcmp(&SIGNATURE, &pk.dat.signature, sizeof(SIGNATURE)) != 0) {
    /*Serial.print("\nERR: Signature mismatch: ");
    Serial.print(SIGNATURE);
    Serial.print(" != ");
    Serial.print(pk.dat.signature);
    Serial.println();    */
    pk.err = e_protocol_err::SIGNATURE_FAIL;
    return false;
  }
  
  pk.signal_strength = raw.signal_strength;
  pk.snr = raw.snr;
  pk.err = e_protocol_err::NONE;
  
  //Serial.printf("Got! SIG: %i SNR: %i\n", pk.signal_strength, pk.snr);
  
  return true;
}
