#include "as_host.h"

/*
Tarefas do host:
- Enviar periodicamente informações processadas
- Responder a requests em outras horas
*/

DHT dht(SENSOR_PIN, SENSOR_TYPE);
bool h_quickanswer = true; // should send data
bool h_quickping = false; // should ping

void as_host()
{
    mprint("======== > Starting as HOST < ========\n");

    //for(auto& i : g_data.rain_prob) {
    //    i = (100 + (get_time_ms() % 800)) * 0.001f;
    //    delay(random(10, 100));
    //}

    set_led(true);
    dht.begin();
    sleep_for(250);
    set_led(false);


    create_task([](void*){while(1) { AutoTiming at(1000); display.draw(); }}, "DrawingTHR", 0, 16384, nullptr, 1);
    create_task([](void*){while(1) { AutoTiming at(both_ping_tempo); h_quickping = true; }}, "QuickPing");
    create_task([](void*){while(1) { AutoTiming at(host_rebroadcast_auto); h_quickanswer = true; }}, "QuickAwAUTO");
    create_task([](void*){while(1) { 
        AutoTiming at(2500); 
        {  
            //sleep_for(300);
            calc_chuva_hora(dht.readTemperature(), dht.readHumidity() * 0.01f);
            //h_quickanswer = true;
        }
    }}, "TempUmidSS", 500, 16384, nullptr, 1);

    while(1) {        
        sleep_for(4000);

        if (h_quickanswer) {
            // send lora
            alg_data cpy = g_data.last_data[0];

            const auto dat = prot_make_forecast(cpy.temp, cpy.umid, g_data.rain_prob);

            h_quickanswer = !LR.send((char*)&dat, sizeof(dat)); // if failed (false), quickanswer again.
            
            if (h_quickanswer) {
                mprint("[LORA] Failed to broadcast forecast.\n");
            }
            else {
                mprint("[LORA] Sent forecast to neighbors successfully.\n");
            }

            continue;
        }
        else if (h_quickping) {
            const auto dat = prot_make_ping();
            if (!LR.send((char*)&dat, sizeof(dat))) {
                mprint("[LORA] Failed sending ping.\n");
            }
            else {
                mprint("[LORA] Sent ping.\n");
            }
            h_quickping = false;
            continue;
        }

        auto pak = LR.pop();
        if (pak && pak.ptr_len == sizeof(prot_bomb)) {
            const prot_bomb& pb = (*(prot_bomb*)pak.ptr.get());
            switch(pb.type) {
            case prot_type::REQUEST_INFO:
            {
                mprint("[LORA] Someone requested info early.\n");
                h_quickanswer = true;
                
                g_data.last_lora_actitivy = get_time_ms();
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