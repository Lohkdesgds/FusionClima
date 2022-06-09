#pragma once

#include <Arduino.h>
#include <heltec.h>
#include <DHT.h>

#include "debug_wrapper.h"
#include "easybattery.h"
#include "cpuctl.h"
#include "displayer.h"
#include "protocollora.h"
#include "algorithm_chuva.h"
#include "shared_data.h"

#define SENSOR_PIN 32
#define SENSOR_TYPE DHT22

/*
Tarefas do host:
- Enviar periodicamente informações processadas
- Responder a requests em outras horas
*/

void as_host();