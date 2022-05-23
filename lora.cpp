#include "lora.h"

namespace LoRaAsync {
  
  char* __cast_begin_p(__pack_dbg_insert* p)
  {
    return (char*)p;
  }
  
  char* __cast_end_p(__pack_dbg_insert* p)
  {
    return ((char*)p) + sizeof(__pack_dbg_insert);
  }
  
  uint64_t lora_get_mac()
  {
    if (!__strlora.has_initialized) lora_init();
    return ESP.getEfuseMac();
  }
  
  std::string lora_get_mac_str()
  {
    if (!__strlora.has_initialized) lora_init();
    uint64_t chipId = lora_get_mac();
    char tmp[24];
    sprintf(tmp, "%04X%08X", (uint16_t)(chipId>>32), (uint32_t)chipId);
    return tmp;
  }

  void __lora_on_receive(int packsiz)
  {
    Serial.printf("Got something of %i\n", packsiz);
    __pack_dbg_insert _insr;
    pack pk;
    
    if(packsiz <= sizeof(_insr)){ // pack is empty or not valid by standard
      Serial.printf("Failed: packsiz too small\n");
      while(--packsiz >= 0) LoRa.read(); // free up
      return;
    }    

    for(size_t p = 0; p < sizeof(_insr) && (--packsiz > 0); ++p) ((char*)&_insr)[p] = (char)LoRa.read();
    if (packsiz == 0) {
      Serial.printf("Failed: packsiz got too small (2)\n");
      return;
    }
    while(--packsiz >= 0) pk.data.push_back((char)LoRa.read());
    
    if (pk.data.empty()) {
      Serial.printf("Data is empty!\n");
      return; // empty packet
    }
    if (memcmp(_insr.SIGNATURE_OFFSET, THIS_SIGNATURE, SIGNATURE_LEN) != 0) { // not a valid packet.
      Serial.printf("Signature mismatch: %s != %s\n", _insr.SIGNATURE_OFFSET, THIS_SIGNATURE);
      while(--packsiz >= 0) LoRa.read(); // free up
      return;
    }
    
    pk.rssi = -LoRa.packetRssi();
    pk.snr = LoRa.packetSnr();
    
    std::lock_guard<std::mutex> l(__strlora.packs_mtx);
    if (__strlora.func_deal) {
      __strlora.func_deal(pk);
    }
    else {
      Serial.printf("No func!\n");
    }
  }
  
  void lora_init(char a, char b, char c, char d)
  {
    if (__strlora.has_initialized) return;
    THIS_SIGNATURE[0] = a;
    THIS_SIGNATURE[1] = b;
    THIS_SIGNATURE[2] = c;
    THIS_SIGNATURE[3] = d;
    LoRa.begin(BAND, true);
    LoRa.setSyncWord(SYNC_WORD);
    LoRa.onReceive(__lora_on_receive);
    LoRa.receive();
    __strlora.has_initialized = true;
  }
  
  void lora_stop()
  {
    if (!__strlora.has_initialized) return;
    LoRa.end();
    __strlora.has_initialized = false;
  }
  
  void lora_hook_recv(std::function<void(pack&)> f)
  {
    if (!__strlora.has_initialized) lora_init();
    std::lock_guard<std::mutex> l(__strlora.packs_mtx);
    __strlora.func_deal = f;
  }
  
  void lora_unhook_recv()
  {
    std::lock_guard<std::mutex> l(__strlora.packs_mtx);
    __strlora.func_deal = {};
  }

  bool lora_send(const char* dat, size_t len)//, uint32_t rebroadcast_radius)
  {
    return lora_send(std::vector<char>(dat, (const char*)(dat + len)));//, rebroadcast_radius);
  }
  
  bool lora_send(std::vector<char> dat)//, uint32_t rebroadcast_radius)
  {
    if (!__strlora.has_initialized) return false;
    if (!LoRa.beginPacket()) return false;
    LoRa.setTxPower(POWER, RF_PACONFIG_PASELECT_PABOOST); // max

    __pack_dbg_insert _insr;
    for(size_t p = 0; p < SIGNATURE_LEN; ++p) _insr.SIGNATURE_OFFSET[p] = THIS_SIGNATURE[p];
    
    dat.insert(dat.begin(), __cast_begin_p(&_insr), __cast_end_p(&_insr));

    Serial.print("Sending: \"");
    for(const auto& ch : dat) Serial.print(ch);
    Serial.printf("\" (size: %zu)\n", dat.size());
    
    size_t _sent = LoRa.write((uint8_t*)dat.data(), dat.size());
    const bool gud = LoRa.endPacket() && _sent == dat.size();
    LoRa.receive();
    return gud;
  }

}
