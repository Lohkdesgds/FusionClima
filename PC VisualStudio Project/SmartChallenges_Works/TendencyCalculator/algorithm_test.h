#pragma once

#include <iostream>

#include "../shared/datahandler.h"

constexpr size_t num_ks = 3;

double chances_of(const pairing8&, const double(&Ks)[num_ks]);