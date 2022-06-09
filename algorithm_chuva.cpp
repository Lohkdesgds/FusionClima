#include "algorithm_chuva.h"

void calc_chuva_hora(float temp, float umid)
{
    // fixo
    {
        mprint("[DEBUG] Read temp/humid: %.2f %.2f\n", temp, 100.0f * umid);
        for(size_t p = alg_history_size - 1; p > 0; --p) {
            g_data.last_data[p] = g_data.last_data[p-1];
            g_data.rain_prob[p] = g_data.rain_prob[p-1];
        }
        g_data.last_data[0].temp = temp;
        g_data.last_data[0].umid = umid;
    }
    // easy use
    float& chuva_chances = g_data.rain_prob[0];
    const float& ult_temperatura = g_data.last_data[1].temp;
    const float& ult_umidade = g_data.last_data[1].umid;
    const float& temperatura = g_data.last_data[0].temp;
    const float& umidade = g_data.last_data[0].umid;

    const auto limfact = [](const float f){ return f > 1.0f ? 1.0f : (f < 0.0f ? 0.0f : f); };

    chuva_chances = 
        limfact(powf(0.3f * limfact(umidade > 0.95f ? ((umidade - 0.92f) / 0.08f) : 0.0f) +
        0.7f * limfact(
            limfact(ult_temperatura < temperatura ? ((temperatura - ult_temperatura) * 1.0f / 1.5f) : 0.0f) * 0.7f +
            limfact(umidade > 0.8f ? ((umidade - 0.8f) / 0.2f) : 0.0f) * 0.3f) +
        0.4f * limfact(
            0.4f * (g_data.last_data[0].umid >= g_data.last_data[1].umid) + 0.25f * (g_data.last_data[1].umid >= g_data.last_data[2].umid) + 0.15f * (g_data.last_data[2].umid >= g_data.last_data[3].umid) +
            0.2f * (g_data.last_data[0].temp <= g_data.last_data[1].temp) + 0.15f * (g_data.last_data[1].temp <= g_data.last_data[2].temp) + 0.10f * (g_data.last_data[2].temp <= g_data.last_data[3].temp)
        ), 0.8f));


    //chuva_chances = random(0, 1000) * 1.0f / 1000.0f;
}