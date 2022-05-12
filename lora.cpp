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
    
    {
      //Serial.print("PACK|");
    
      std::lock_guard<std::mutex> l(_g_lora.mtx);
      
      //Serial.print("LCK|");
      
      /*if (_g_lora.packs.size() >= lora_max_packs) {
        Serial.print("OVERFLOW_DISCARD\n");
        for (int i = 0; i < packsiz; i++) LoRa.read(); // discard packet unfortunately.
        return;
      }*/
          
      //Serial.print("MEM|");
      //package pk;
      
      //Serial.print("CPY|");
      
      while (LoRa.available()) {
        _g_lora.last_pack.str += (char)LoRa.read();
      }
      
      //Serial.print("RSSI|");
      _g_lora.last_pack.signal_strength = LoRa.packetRssi();
      
      //Serial.print("SNR|");
      _g_lora.last_pack.snr = LoRa.packetSnr();

      _g_lora.has_pack = true;
      
      //Serial.print("MOV|");
      //_g_lora.packs.push_back(std::move(pk));
      
      //Serial.print("END|");
    }
    
    //Serial.print("GOOD\n");
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
    size_t _sent = LoRa.write((uint8_t*)dat, len);
    const bool gud = LoRa.endPacket() && _sent == len;    
    LoRa.receive();
    return gud;
  }
  
  bool lora_pop_recv(package& pk)
  {
    /*std::lock_guard<std::mutex> l(_g_lora.mtx);
    if (_g_lora.packs.empty()) return false;
    
    pk = std::move(_g_lora.packs.front());
    _g_lora.packs.erase(_g_lora.packs.begin());
    return true;*/
    std::lock_guard<std::mutex> l(_g_lora.mtx);
    if (!_g_lora.has_pack) return false;
    _g_lora.has_pack = false;
    pk = std::move(_g_lora.last_pack);
    return true;
  }
  
  size_t lora_recv_length()
  {
    //return _g_lora.packs.size();
    return _g_lora.has_pack ? 1 : 0;
  }

  /*void _i_lora_recv(uint8_t* payload, uint16_t len, int16_t rssi, int8_t snr)
  {
    if (_g_lora.packs.size() >= lora_max_packs) return;
    package pk;
    
    pk.pkg.insert(pk.pkg.begin(), (char*)payload, ((char*)payload + (size_t)len));
    pk.signal_strength = rssi;
    pk.snr = snr;

    std::lock_guard<std::mutex> l(_g_lora.mtx_recv);
    _g_lora.packs.push_back(std::move(pk));
  }
  
  void _i_lora_tx_good()
  {
    std::lock_guard<std::mutex> l(_g_lora.mtx_send);
      _g_lora.send_news = true;
      _g_lora.send_confirm = false;
  }
  
  void _i_lora_tx_bad()
  {
    std::lock_guard<std::mutex> l(_g_lora.mtx_send);
      _g_lora.send_news = true;
      _g_lora.send_confirm = false;
  }
  
  void lora_init(uint32_t licensing[4])
  {  
    SPI.begin(SCK,MISO,MOSI,SS);
    Mcu.init(SS,RST_LoRa,DIO0,DIO1,licensing);  

    RadioEvents.TxDone = _i_lora_tx_good;
    RadioEvents.TxTimeout = _i_lora_tx_bad;
    RadioEvents.RxDone = _i_lora_recv;

    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
  } 
  
  bool lora_pop_recv(package& pk)
  {
    std::lock_guard<std::mutex> l(_g_lora.mtx_recv);
    if (_g_lora.packs.empty()) return false;
    
    pk = std::move(_g_lora.packs.front());
    _g_lora.packs.erase(_g_lora.packs.begin());
    return true;
  }
  
  bool lora_send(char* dat, size_t len)
  {
    {
      std::lock_guard<std::mutex> l(_g_lora.mtx_send);
      _g_lora.send_confirm = _g_lora.send_news = false;
    }

    _g_lora.stat = lora_current_stat::LORA_SEND;
    Radio.Send((uint8_t*)dat, len);
    for(size_t _c = 0; _c < 20 && !_g_lora.send_news; ++_c) std::this_thread::sleep_for(std::chrono::milliseconds(50));

    _g_lora.stat = lora_current_stat::LORA_RECV;
    Radio.Rx(0);
    return _g_lora.send_confirm;
  }*/

}
