#pragma once

#include <Arduino.h>
#include "DHT.h"
#include <mutex>

#include "lora.h"
#include "cpu_tools.h"
#include "calculador.h"

#define SENSOR_PIN 32
#define SENSOR_TYPE DHT22

extern DHT dht;

namespace Protocol {
    
    // Types of requests (both)
    enum class prot_type : uint32_t { NONE, REQUEST_INFO, UPDATE, PING };

    // Request type: REQUEST (client requests new data)
    struct prot_request {
        bool dummy;
    };

    // Request type: POST (new info to users)
    struct prot_data {
        float curr_temp = 0.0f, curr_umid = 0.0f;
        float perc_willrain =  0.0f;
    };

    // Final block
    struct prot_bomb {
        prot_type type = prot_type::NONE;
        union {
            prot_request request;
            prot_request ping;
            prot_data forecast;
        } data{};
    };
    
    constexpr uint64_t prot_client_time_to_request = 90 * 1000; // ms (request if time > ...)
    constexpr uint64_t prot_host_time_send_at = 60 * 1000; // ms (send each ...)
    
    class SyncCtl {
        mutable std::mutex safe_send;
        uint64_t m_last_doe = 0;
        prot_data m_last_data{};
    public:
        // CLIENT STUFF:
        void client_auto_request(); // request if time is 0 or timenow - that > prot_client_time_to_request
        
        // HOST STUFF:
        void host_auto_post(); // if time, send
        bool host_manual_post(); // if someone ask for data, send
        
        // GENERAL
        bool any_ping() const;
        const prot_data& get() const; // for display data or manual update
        uint64_t get_delta_time() const; // ms
        
        void update(prot_data ndat, bool settime);
    };
}

extern Protocol::SyncCtl Syncer;

/*
struct protocol {
    uint64_t device = 0;
    float temp = 0.0f, temp_d = 0.0f;
    float humd = 0.0f, humd_d = 0.0f;
};

struct protocol_extra {
    protocol dat;
    int signal_strength = 0; // typ: [0, -120] [dB]
    float snr = 0; // typ: [-20, 10] [dB]
};

bool send_pack(protocol&);
bool try_get_pack(protocol_extra&);
*/