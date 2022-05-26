#include "lora.h"
    
LoRaC::packctl LR;
    
namespace LoRaC {
    
    pack::operator bool() const
    {
        return ptr_len > 0 && ptr.get() != nullptr;
    }
    
    bool packctl::begin(uint16_t nkey, long freq)
    {
        key = nkey;
        if (ready) return true;
        if (!LoRa.begin(915E6, true)) return false;
        LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
        //LoRa.setSignalBandwidth(250E3); // lower == better range, cost: bitrate | opts: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3
        //LoRa.setSpreadingFactor(7);
        //LoRa.setCodingRate4(8); // 5 to 8, 8 has more error check I think
        //LoRa.setPreambleLength(32);
        //LoRa.enableInvertIQ();
        //LoRa.setSyncWord(0x01);
        LoRa.onReceive(__lora_receive);
        LoRa.receive();
        ready = true;
        return true;
    }
    
    void packctl::end()
    {
        if (!ready) return;
        LoRa.end();
        ready = false;
    }
    
    bool packctl::__push(pack&& mov)
    {
        if (packs.size() >= max_packs) return false;
        std::lock_guard<std::mutex> l(packs_mtx);
        packs.emplace_back(std::move(mov));
        return true;
    }
    
    bool packctl::has() const
    {
        return packs.size() > 0;
    }
    
    packctl::operator bool() const
    {
        return packs.size() > 0;
    }
    
    float packctl::currentSnr() const
    {
        return LoRa.packetSnr();
    }
    
    int32_t packctl::currentRssi() const
    {
        return LoRa.packetRssi();
    }
    
    uint16_t packctl::get_key() const
    {
        return key;
    }
    
    bool packctl::has_begun() const
    {
        return ready;
    }
    
    pack packctl::pop()
    {
        if (packs.empty()) return {};
        pack p;
        std::lock_guard<std::mutex> l(packs_mtx);
        p = std::move(packs.front());
        packs.pop_front();
        return p;
    }
    
    bool packctl::send(char* buf, size_t len)
    {
        if (!buf || len == 0) return false;
        if (!LoRa.beginPacket()) {
            LoRa.receive();
            return false;
        }
        LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
        LoRa.write((uint8_t*)&key, sizeof(key));
        LoRa.write((uint8_t*)buf, len);
        const bool gud = LoRa.endPacket();
        LoRa.receive();
        return gud;
    }
    
    void __lora_receive(const int len)
    {
        Serial.printf("[LoRa] Received: %i\n", len);
        
        rawpackage wrk;
        if (len < sizeof(wrk.key)) {
            Serial.printf("[LoRa] Packet was too small: %d\n", len);
            return;
        }
        
        wrk.rest.ptr_len = len - sizeof(wrk.key);
        wrk.rest.ptr = std::unique_ptr<char[]>(new char[wrk.rest.ptr_len]);
        
        LoRa.readBytes((char*)&wrk.key, sizeof(wrk.key));
        char* damnit = wrk.rest.ptr.get();
        LoRa.readBytes(damnit, wrk.rest.ptr_len);
        
        if (LR.get_key() != wrk.key) {
            Serial.printf("[LoRa] Packet didn't match key: %u != %u\n", (unsigned)LR.get_key(), (unsigned)wrk.key);
            return;
        }        
        
        if (!LR.__push(std::move(wrk.rest))) {
            Serial.printf("[LoRa] Failed to push received package.\n");
        }
    }
}