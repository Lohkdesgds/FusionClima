#pragma once

#include "shared/tools.h"
#include "shared/buttons.h"
#include "shared/display.h"
#include "shared/import/DHT.h"
#include "shared/import/LoRa.h"
#include "shared/wifictl.h"
#include "shared/cpuctl.h"

inline const char HOSTTAG[] = "MAINHST";
inline const char CLITAG[] = "MAINCLI";

// SHARED:
inline LoRa* lora = nullptr;


// HOST only:
enum class host_menus{ QRCODE, TEMPERATURE, LORA, NONE, _MAX };

inline host_menus _menu = host_menus::QRCODE;
inline qrcodegen::QrCode* qr = nullptr;
inline DHT* hdt = nullptr;
inline uint64_t last_click = 0;