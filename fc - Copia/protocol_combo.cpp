#include "protocol_combo.h"

namespace Comm {
  const char protID[] = "[ELRAINF] ";
#ifdef ESP_PLATFORM 
  const uint64_t lora_mac = ESP.getEfuseMac();
  const uint8_t lora_syncword = 0x7F;
#endif
}