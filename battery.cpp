#include "battery.h"

battinf _battinf;

void setup_battery()
{
    if (_battinf._started) return;
    pinMode(VBATT_READ_PIN, OPEN_DRAIN);
    pinMode(VEXT_ENABLE_PIN, OUTPUT);
    digitalWrite(VEXT_ENABLE_PIN, LOW);
    _battinf._started = true;
}

float read_battery_perc(bool limit_range)
{
    if (!_battinf._started) setup_battery();
    //digitalWrite(VEXT_ENABLE_PIN, LOW);
    delay(10);  
    float sum = 0.0f;
    for(unsigned p = 0; p < VBATT_SMOOTH_DEF; ++p) {
        delay(15);
        sum += analogRead(VBATT_READ_PIN);
    }
    //digitalWrite(VEXT_ENABLE_PIN, HIGH);  

    sum /= 1.0f * VBATT_SMOOTH_DEF;

    float _val = (map((long)(sum * (float)VBATT_PRECISION), 1050 * VBATT_PRECISION, 1375 * VBATT_PRECISION, 0, 100 * VBATT_PRECISION) * 1.0f / VBATT_PRECISION);

    Serial.printf("RAW BATT: %.1f -> %.2f%%\n", sum, _val);
    
    if (_battinf.last_read < -50.0f) {
        _battinf.last_read = _battinf.last_read_smooth = _val;
    }
    else {
        _battinf.last_read = _val;
        _battinf.last_read_smooth = (_battinf.last_read_smooth * VBATT_SMOOTH + _battinf.last_read) * 1.0f / (VBATT_SMOOTH + 1.0f);
    }

    if (limit_range && _val < 0.0f) _val = 0.0f;
    if (limit_range && _val > 100.0f) _val = 100.0f;
    return _val;
}

float read_battery_perc_cache(bool limit_range)
{
    float _val = _battinf.last_read;
    if (limit_range && _val < 0.0f) _val = 0.0f;
    if (limit_range && _val > 100.0f) _val = 100.0f;
    return _val;
}