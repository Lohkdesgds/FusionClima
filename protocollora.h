#pragma once

#include <Arduino.h>
#include <heltec.h>

#include "shared_data.h"

// expects return
prot_bomb prot_make_request();

// does not expect return
prot_bomb prot_make_ping();

// for a return
prot_bomb prot_make_forecast(float temp, float umid, const float (&willrain)[alg_history_size]);