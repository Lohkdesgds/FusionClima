#include "algorithm_chuva.h"

void calc_chuva_hora(float temp, float umid)
{
    g_data.last_data[0].temp = temp;
    g_data.last_data[0].umid = umid;

    g_data.rain_prob[0] = chuva_chances();

    // fixo
    //mprint("[DEBUG] Read temp/humid: %.2f %.2f\n", temp, 100.0f * umid);
    if (g_data.last_forecast_update == 0) {
        g_data.last_forecast_update = get_time_ms();
        for(size_t p = alg_history_size - 1; p > 0; --p) {
            g_data.last_data[p].temp = temp;
            g_data.last_data[p].umid = umid;
            g_data.rain_prob[p] = 0.0;
        }
    }
    else if (get_time_ms() - g_data.last_forecast_update >= host_hora) {
        g_data.last_forecast_update = get_time_ms();
        for(size_t p = alg_history_size - 1; p > 0; --p) {
            g_data.last_data[p] = g_data.last_data[p-1];
            g_data.rain_prob[p] = g_data.rain_prob[p-1];
        }
    }
}

float chuva_chances()
{
    // easy use
    //float& chuva_chances = g_data.rain_prob[0];
    const float& ult_temperatura = g_data.last_data[1].temp;
    const float& ult_umidade = g_data.last_data[1].umid;
    const float& temperatura = g_data.last_data[0].temp;
    const float& umidade = g_data.last_data[0].umid;

    const auto limfact = [](const float f){ return f > 1.0f ? 1.0f : (f < 0.0f ? 0.0f : f); };

    return 
        limfact(
            powf(
                0.18f * limfact(umidade > 0.92f ? powf((umidade - 0.92f) / 0.08f, 1.7f) : 0.0f) +
                0.35f * limfact(
                    limfact(ult_temperatura > temperatura ? ((ult_temperatura - temperatura) * 1.0f / 1.5f) : 0.0f) * 0.7f +
                    limfact(umidade > 0.8f ? ((umidade - 0.8f) / 0.2f) : 0.0f) * 0.4f) +
                0.55f * powf(limfact(
                    0.36f * (g_data.last_data[0].umid >= g_data.last_data[1].umid) + 0.17f * (g_data.last_data[1].umid >= g_data.last_data[2].umid) + 0.08f * (g_data.last_data[2].umid >= g_data.last_data[3].umid) +
                    0.25f * (g_data.last_data[0].temp <= g_data.last_data[1].temp) + 0.10f * (g_data.last_data[1].temp <= g_data.last_data[2].temp) + 0.05f * (g_data.last_data[2].temp <= g_data.last_data[3].temp)
                ), 0.4f)
                - 0.15f * powf(limfact(
                    0.60f * (g_data.last_data[0].umid <= g_data.last_data[1].umid) + 0.10f * (g_data.last_data[1].umid <= g_data.last_data[2].umid) +
                    0.20f * (g_data.last_data[0].temp >= g_data.last_data[1].temp) + 0.10f * (g_data.last_data[1].temp >= g_data.last_data[2].temp)
                ), 0.8f)
            , 1.3f)
        );


    //chuva_chances = random(0, 1000) * 1.0f / 1000.0f;
}