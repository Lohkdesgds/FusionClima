#include "calculador.h"

float hist_temps[24]{};
float hist_umids[24]{};
size_t ponto = 0;

float calcular_chance(float temp, float umid)
{
    return 0.67f; // retornar as chances de chuva.
}