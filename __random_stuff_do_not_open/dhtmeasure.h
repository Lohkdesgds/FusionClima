#pragma once

#include <heltec.h>
#include <DHT.h>

#include "tools.h"
#include "data.h"
#include "calc.h"

#define DHTTYPE DHT22 

extern DHT* __dht;

void dht_begin(uint32_t pin);
void __dht_read(float& temp, float& umid);
void __dht_loop(void*);