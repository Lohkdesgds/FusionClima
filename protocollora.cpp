#include "protocollora.h"

// expects return
prot_bomb prot_make_request()
{
    prot_bomb b;
    b.type = prot_type::REQUEST_INFO;
    b.data.request.dummy = true;
    return b;
}

// does not expect return
prot_bomb prot_make_ping()
{
    prot_bomb b;
    b.type = prot_type::PING;
    b.data.ping.dummy = true;
    return b;
}

// for a return
prot_bomb prot_make_forecast(float temp, float umid, const float (&willrain)[alg_history_size])
{
    prot_bomb b;
    b.type = prot_type::UPDATE;
    b.data.forecast.curr_temp = temp;
    b.data.forecast.curr_umid = umid;
    for(size_t p = 0; p < alg_history_size; ++p) {
        b.data.forecast.perc_willrain[p] = willrain[p] * 255.0f;
    }
    return b;
}