#include "lora.h"

namespace LoRaAsync {

  package::~package()
  {
  }
  
  package::package(package&& p)
  {
    str = std::move(p.str);
    signal_strength = p.signal_strength;
    snr = p.snr;
  }
  
  void package::operator=(package&& p)
  {
    str = std::move(p.str);
    signal_strength = p.signal_strength;
    snr = p.snr;
  }
  
  void package::write(const char* dat, size_t ln)
  {
    str.clear();
    for(size_t p = 0; p < ln; ++p) str += dat[p];
  }
  
  size_t package::size() const
  {
    return str.length();
  }
  
  const char* package::data() const
  {
    return str.c_str();
  }
  
  uint64_t lora_get_mac()
  {
    return ESP.getEfuseMac();
  }
  
  std::string lora_get_mac_str()
  {
    uint64_t chipId = lora_get_mac();
    char tmp[24];
    sprintf(tmp, "%04X%08X", (uint16_t)(chipId>>32), (uint32_t)chipId);
    return tmp;
  }

  void lora_on_receive(int packsiz)
  {
    if (packsiz == 0) return;

    std::lock_guard<std::mutex> l(_g_lora.mtx);
    
    while (LoRa.available()) {
      _g_lora.last_pack.str += (char)LoRa.read();
    }
    _g_lora.last_pack.signal_strength = LoRa.packetRssi();
    _g_lora.last_pack.snr = LoRa.packetSnr();

    _g_lora.has_pack = true;
  }

  void lora_init()
  {
    if (_g_lora.keep_recv_b) return;
    LoRa.begin(BAND, true);
    LoRa.setSyncWord(SYNC_WORD);
    _g_lora.keep_recv_b = true;
    _g_lora.has_pack = false;
    LoRa.onReceive(lora_on_receive);
    LoRa.receive();
  }

  void lora_stop() {
    if (!_g_lora.keep_recv_b) return;
    _g_lora.keep_recv_b = false;
    LoRa.end();
  }

  bool lora_send(char* dat, size_t len)
  {
    if (!_g_lora.keep_recv_b) {
      return false;
    }
    std::lock_guard<std::mutex> l(_g_lora.mtx);
    if (!LoRa.beginPacket()) return false;
    LoRa.setTxPower(POWER, RF_PACONFIG_PASELECT_PABOOST); // max
    size_t _sent = LoRa.write((uint8_t*)dat, len);
    const bool gud = LoRa.endPacket() && _sent == len;    
    LoRa.receive();
    return gud;
  }
  
  bool lora_pop_recv(package& pk)
  {
    std::lock_guard<std::mutex> l(_g_lora.mtx);
    if (!_g_lora.has_pack) return false;
    _g_lora.has_pack = false;
    pk = std::move(_g_lora.last_pack);    
    return true;
  }
  
  size_t lora_recv_length()
  {
    return _g_lora.has_pack ? 1 : 0;
  }
}
