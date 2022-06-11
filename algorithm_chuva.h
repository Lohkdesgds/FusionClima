#pragma once

#include <Arduino.h>
#include <heltec.h>

#include "debug_wrapper.h"
#include "shared_data.h"
#include "cpuctl.h"

void calc_chuva_hora(float temp, float umid);

float chuva_chances();