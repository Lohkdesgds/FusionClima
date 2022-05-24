#pragma once

#include "heltec.h"
#include <deque>
#include <mutex>
#include <memory>

// incl
namespace LoRaC {
    
    const size_t max_packs = 20;
    const uint64_t lora_mac_this = ESP.getEfuseMac();
    
    struct pack {
        std::unique_ptr<char[]> ptr;
        size_t ptr_len;
        
        operator bool() const;
    };

    struct rawpackage {
        uint16_t key;
        pack rest;
    };
    
    class packctl {
        std::deque<pack> packs;
        std::mutex packs_mtx;
        uint16_t key = 0;
        bool ready = false;
    public:
        bool begin(uint16_t nkey, long freq = 915E6);
        void end();
    
        bool __push(pack&&);
        
        bool has() const;
        operator bool() const;
        
        float currentSnr() const;
        int32_t currentRssi() const;
        
        uint16_t get_key() const;
        
        bool has_begun() const;
        
        pack pop();
        bool send(char*, size_t);
    };

    extern packctl LR;
    
    void __lora_receive(const int);
}
