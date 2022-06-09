#include "calc.h"

float calc_call(const float tmp, const float umd)
{
    const float rainn = calc_algorithm(tmp, umd, __rawshrdata.m_temp_avg, __rawshrdata.m_umid_avg);
    __rawshrdata.data_add(tmp, umd, rainn);
    return rainn;
}

float calc_algorithm(const float& temp_agora, const float& umid_agora, const float& temp_avg, const float& umid_avg)
{
    static float percsmooth = 0.5f;
    percsmooth = (percsmooth * 9.0f + (random(0, 1000) * 0.001f)) * 1.0f / 10.0f;
    return percsmooth;
}