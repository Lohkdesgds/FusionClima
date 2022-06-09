#pragma once

#include <Arduino.h>
#include <heltec.h>

#include "debug_wrapper.h"
#include "benchmark.h"
#include "cpuctl.h"
#include "easybattery.h"
#include "displayer.h"
#include "protocollora.h"
#include "shared_data.h"
#include "button.h"

/*
Tasks do cliente:
- Monitorar bateria
- Receber / requisitar dados
- Mostrar dados na tela
*/

void as_client();