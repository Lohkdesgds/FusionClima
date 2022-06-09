#pragma once

#include <heltec.h>

#include "tools.h"

constexpr float data_avg_coef_temp = 2.0f; // peso no valor anterior (novo + anterior * this) / (this + 1)
constexpr float data_avg_coef_umid = 2.0f; // peso no valor anterior (novo + anterior * this) / (this + 1)
constexpr uint8_t data_graph_size_each = 8; // NÃO MUDAR (cabe 8 por byte)
constexpr uint8_t data_graph_size_fixed = 16 * data_graph_size_each; // NÃO MUDAR

constexpr uint64_t data_time_to_send_next = 1000 * 10; // 10 seg por agora

struct shared_data {
    uint32_t m_graph[data_graph_size_fixed / data_graph_size_each]; // 32 bytes, [0..31, 32..63, 64..127, 128..159, 160..191, 192..223, 224..255]
    int16_t m_temp, m_umid, m_rainchance;
    uint8_t m_graph_last; // iterator
};

struct request_model {
    uint8_t request_type = 0; // [1 == REPOST]
};

struct raw_shared_data {
    uint64_t m_last_send = 0;
    float m_temp = 20.0f, m_umid = 50.0f, m_rainchance = 50.0f;
    float m_temp_avg = 0.0f, m_umid_avg = 0.0f;
    float m_battlevel = 0.0f;
    int32_t m_last_rssi = 0;

    uint32_t m_graph[data_graph_size_fixed / data_graph_size_each]{}; // chances de chuva graph
    uint8_t m_graph_it = 0;

    void data_add(const float temp, const float umid, const float rain);

    operator shared_data() const;
    void import(const shared_data&);

    float conv_data_pt_rain(const uint8_t);
};

extern raw_shared_data __rawshrdata;
