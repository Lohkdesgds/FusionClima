#include "dhtmeasure.h"

DHT* __dht = nullptr;

void dht_begin(uint32_t pin)
{
    if (__dht) {
        mprint("[DHT] Already loaded.\n");
        return;
    }
    __dht = new DHT(pin, DHTTYPE);
    __dht->begin();

    xTaskCreatePinnedToCore(__dht_loop, "DHTAuto", 3072, nullptr, 500, nullptr, 0);
    mprint("[DHT] Created DHT object and thread.\n");
}

void __dht_read(float& temp, float& umid)
{
    umid = __dht->readHumidity() * 0.01f;
    temp = __dht->readTemperature();
}

void __dht_loop(void*)
{    
    mprint("[DHT] Task measurer up!\n");
    while(1) {
        delay(2100);
        float t, u;
        __dht_read(t, u);
        calc_call(t, u);
        yield();
    }
}