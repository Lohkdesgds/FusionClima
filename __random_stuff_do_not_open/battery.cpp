#include "battery.h"


void battery_begin()
{
    pinMode(pin_batt, OPEN_DRAIN);
    pinMode(pin_vext, OUTPUT);

    digitalWrite(pin_vext, LOW);

    xTaskCreatePinnedToCore(__battery_loop, "AsyncBatt", 3072, nullptr, tskIDLE_PRIORITY, nullptr, 1);    
}

void __battery_loop(void*)
{
    float __battlevel = 0.5f;
    mprint("[BATT] Monitoring battery.\n");

    while(1)
    {
        //digitalWrite(pin_vext, LOW);
        delay(10);
        const auto rred = analogRead(pin_batt);
        //digitalWrite(pin_vext, HIGH);

        const float val = (map((long)(rred * (float)vbatt_precision), 1050 * vbatt_precision, 1375 * vbatt_precision, 0, 100 * vbatt_precision) * 0.01f / vbatt_precision);

        __battlevel = (__battlevel * batt_smooth_cte + val) * 1.0f / (1.0f + batt_smooth_cte);

        __rawshrdata.m_battlevel = __battlevel > 1.0f ? 1.0f : (__battlevel < 0.0f ? 0.0f : __battlevel);

        delay(2000);
    }
}