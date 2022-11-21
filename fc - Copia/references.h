#pragma once

constexpr uint8_t pin_SWITCH = 17;
const uint8_t pin_LED = 25;

#define SLED(VALUE) digitalWrite(pin_LED, VALUE)
#define IS_DONGLE() digitalRead(pin_SWITCH)
#define IS_MONITOR() (!digitalRead(pin_SWITCH))

#define PRTLN(...) { Serial.printf(__VA_ARGS__); Serial.print('\n'); }
#define AUTOF(...) [&]{ static char _buf[96]; snprintf(_buf, 96, __VA_ARGS__); return _buf; }()
#define DISPLAYBLOCK(...){ m_dsp.clearBuffer(); __VA_ARGS__; m_dsp.sendBuffer(); }

void as_dongle(void*); // as_dongle.cpp
void as_monitor(void*); // as_monitor.cpp
void errcoderst(uint8_t cod); // fc.ino