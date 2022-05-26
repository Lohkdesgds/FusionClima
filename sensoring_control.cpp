#include "sensoring_control.h"

DHT dht(SENSOR_PIN, SENSOR_TYPE);

namespace Protocol {
    
    // CLIENT STUFF:
    void SyncCtl::client_auto_request() // request if time is 0 or timenow - that > prot_client_time_to_request
    {        
        std::lock_guard<std::mutex> l(safe_send);
        //delay(random(2000, 10000));
        
        const uint64_t _t = get_time_ms();
        if (_t - m_last_doe < prot_client_time_to_request && m_last_doe != 0) return;
        
        prot_bomb pak;
        pak.type = prot_type::REQUEST_INFO;
        pak.data.request.dummy = true; // just to set something.
        
        if (!LR.send((char*)&pak, sizeof(pak))) Serial.println("LoRa request info send failed.");
        else Serial.println("LoRa request done.");
    }
    
    // HOST STUFF:
    void SyncCtl::host_auto_post() // if time, send
    {
        // no lock, it's done at host_manual_post()
        const uint64_t _t = get_time_ms();
        if (_t - m_last_doe < prot_host_time_send_at && m_last_doe != 0) return;
        
        if (!host_manual_post()) Serial.println("LoRa forecast update send failed.");
        else {
            Serial.println("LoRa forecast update done.");
            m_last_doe = _t;
        }
    }
    
    bool SyncCtl::host_manual_post() // if someone ask for data, send
    {
        std::lock_guard<std::mutex> l(safe_send);
        //delay(random(2000, 10000));
        
        prot_bomb pak;
        pak.type = prot_type::UPDATE;
        pak.data.forecast = m_last_data;
        
        return LR.send((char*)&pak, sizeof(pak));
    }
    
    // GENERAL
    
    bool SyncCtl::any_ping() const
    {
        std::lock_guard<std::mutex> l(safe_send);
        //delay(random(2000, 10000));
        
        prot_bomb pak;
        pak.type = prot_type::PING;
        pak.data.request.dummy = true; // just to set something.
        
        return LR.send((char*)&pak, sizeof(pak));
    }
    
    const prot_data& SyncCtl::get() const
    {
        return m_last_data;
    }
    
    uint64_t SyncCtl::get_delta_time() const // ms
    {
        return get_time_ms() - m_last_doe;
    }
    
    void SyncCtl::update(prot_data ndat, bool settime)
    {
        m_last_data = ndat;
        if (settime) m_last_doe = get_time_ms();
    }
}

Protocol::SyncCtl Syncer;

/*using namespace LoRaC;

bool send_pack(protocol& pk)
{
    if (!LR.has_begun()) return false;
    pk.device = lora_mac_this;
    return LR.send((char*)&pk, sizeof(pk));
}

bool convert_pack(const std::vector<char> dat, protocol_extra& pk)
{
    pack pak = LR.pop();
    if (!pak || pak.ptr_len != sizeof(pk.dat)) return false;
    
    memcpy((void*)&pk.dat, pak.ptr.get(), sizeof(pk.dat));

    if (pk.dat.device == lora_mac_this) return false; // heard itself?
    pk.signal_strength = LR.currentRssi();
    pk.snr = LR.currentSnr();
    return true;
}*/