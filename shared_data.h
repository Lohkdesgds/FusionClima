#pragma once

#include <heltec.h>

// - - - - - - CONSTANTS
constexpr size_t alg_history_size = 48;
constexpr uint64_t host_hora = /*10000;//*/1000 * 60 * 60; // 1000 ms -> 1 s, 60 s -> 1 min, 60 min -> 1 hora
constexpr uint64_t both_ping_tempo = 1000 * 60 * 2;

// - - - - - - STRUCTS

// Types of requests (both)
enum class prot_type : uint32_t { NONE, REQUEST_INFO, UPDATE, PING };

// Request type: REQUEST (client requests new data)
struct prot_request {
    bool dummy;
};

// Request type: POST (new info to users)
struct prot_data {
    float curr_temp = 0.0f, curr_umid = 0.0f;
    uint8_t perc_willrain[alg_history_size];
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


struct alg_data {
    float temp, umid;
};

struct display_data {
    uint64_t last_lora_actitivy = 0;
    float rain_prob[alg_history_size]{}; // [0] == latest
    alg_data last_data[alg_history_size]{}; // [0] == latest. Client: saves only last
};

extern display_data g_data;