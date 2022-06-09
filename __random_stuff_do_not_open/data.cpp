#include "data.h"

raw_shared_data __rawshrdata;

void raw_shared_data::data_add(const float temp, const float umid, const float rain)
{
    if (isnan(temp) || isnan(umid) || isnan(rain)) {
        mprint("[DATA] NaN detected, avoiding NaN data.\n");
        return;
    }
    m_temp = temp;
    m_umid = umid;
    m_rainchance = rain;

    m_temp_avg = (m_temp_avg * data_avg_coef_temp + temp) * 1.0f / (data_avg_coef_temp + 1.0f);
    m_umid_avg = (m_umid_avg * data_avg_coef_umid + umid) * 1.0f / (data_avg_coef_umid + 1.0f);

    const uint8_t mpt = m_graph_it / data_graph_size_each;
    const uint8_t adv = m_graph_it % data_graph_size_each;

    const uint8_t cnv = static_cast<uint8_t>(rain * 0x20) & 0b1111;

    m_graph[mpt] &= ~(0b1111 << (adv * 4)); // limpa bits
    m_graph[mpt] |= (cnv << (adv * 4)); // define bits
    
    m_graph_it = (m_graph_it + 1) % data_graph_size_fixed;
}

raw_shared_data::operator shared_data() const
{
    shared_data d;
    d.m_temp = 10.0f * m_temp;
    d.m_umid = 10.0f * m_umid;
    d.m_rainchance = 10.0f * m_rainchance;
    memcpy(&d.m_graph, m_graph, sizeof(m_graph));
    d.m_graph_last = m_graph_it;

    return d;
}

void raw_shared_data::import(const shared_data& d)
{
    m_temp = 0.1f * d.m_temp;
    m_umid = 0.1f * d.m_umid;
    m_rainchance = 0.1f * d.m_rainchance;
    memcpy(m_graph, d.m_graph, sizeof(m_graph));
    m_graph_it = d.m_graph_last;
}

float raw_shared_data::conv_data_pt_rain(const uint8_t p)
{
    if (p >= 128) return 0.0f;

    const uint8_t mpt = p / data_graph_size_each;
    const uint8_t adv = p % data_graph_size_each;

    return static_cast<float>((m_graph[mpt] >> (adv * 4)) & 0b1111) / 0b1111;
}