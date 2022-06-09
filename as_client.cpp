#include "as_client.h"

/*
Tasks do cliente:
- Monitorar bateria
- Receber / requisitar dados
- Mostrar dados na tela
*/

bool c_should_request = true;
bool c_quickping = false;

void as_client()
{
    mprint("======== > Starting as CLIENT < ========\n");
    set_led(true);
    sleep_for(10);
    set_led(false);

    create_task([](void*){while(1) { AutoTiming at(1000); display.draw(); }}, "DrawingTHR", 0, 16384, nullptr, 1);
    create_task([](void*){while(1) { AutoTiming at(both_ping_tempo); c_quickping = true; }}, "c_quickping");
    button_begin(0);

    while(1) {
        if (c_should_request) {
            const auto dat = prot_make_request();
            c_quickping = c_should_request = !LR.send((char*)&dat, sizeof(dat));

            if (c_should_request) {
                mprint("[LORA] Failed requesting forecast.\n");
            }
            else {
                mprint("[LORA] Requested forecast.\n");
            }

            continue;
        }
        else if (c_quickping) {
            const auto dat = prot_make_ping();
            if (!LR.send((char*)&dat, sizeof(dat))) {
                mprint("[LORA] Failed sending ping.\n");
            }
            else {
                mprint("[LORA] Sent ping.\n");
            }
            c_quickping = false;
            continue;
        }
        
        auto pak = LR.pop();
        if (pak && pak.ptr_len == sizeof(prot_bomb)) {                
            const prot_bomb& pb = (*(prot_bomb*)pak.ptr.get());
            switch(pb.type) {
            case prot_type::UPDATE:
            {                
                mprint("[LORA] Update received: %.1f C | %.1f%% | Rain? %.1f\n", pb.data.forecast.curr_temp, pb.data.forecast.curr_umid * 100.0f, pb.data.forecast.perc_willrain[0]);

                g_data.last_lora_actitivy = get_time_ms();
                //i.rain_prob = pb.data.forecast.perc_willrain;                
                for(size_t p = 0; p < alg_history_size; ++p) {
                    g_data.rain_prob[p] = static_cast<float>(pb.data.forecast.perc_willrain[p]) / 255.0f;
                }
                g_data.last_data[0].temp = pb.data.forecast.curr_temp;
                g_data.last_data[0].umid = pb.data.forecast.curr_umid;
            }
                break;
            case prot_type::PING:
            {                
                mprint("[LORA] Ping received.\n");

                g_data.last_lora_actitivy = get_time_ms();
            }
                break;
            default:
                break;
            }
        }
    }
}