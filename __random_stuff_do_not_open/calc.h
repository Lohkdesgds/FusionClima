#pragma once

#include <heltec.h>
#include "data.h"

float calc_call(const float, const float);

// recebe 4 constantes, retorna chance chuva
float calc_algorithm(const float& temp_agora, const float& umid_agora, const float& temp_avg, const float& umid_avg);