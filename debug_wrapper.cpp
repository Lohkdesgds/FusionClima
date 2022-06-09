#include "debug_wrapper.h"

__i_dbg __dbg;

__i_dbg::__i_dbg()
{
    pinMode(ESP_MODE_PIN, INPUT_PULLUP);
    pinMode(ESP_LED, OUTPUT);
    Serial.begin(115200);
}

void set_led(const bool b)
{
    digitalWrite(ESP_LED, b);
}

bool get_is_host()
{
    return digitalRead(ESP_MODE_PIN) != HIGH;
}