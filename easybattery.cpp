#include "easybattery.h"

__i_batt __batt;

__i_batt::__i_batt()
{
    pinMode(VBATT_READ_PIN, OPEN_DRAIN);
    pinMode(VEXT_ENABLE_PIN, OUTPUT);
    digitalWrite(VEXT_ENABLE_PIN, LOW);
}

float get_battery_perc(const bool trunc)
{
    //std::lock_guard<std::mutex> l(__batt.vext_mtx);

    //digitalWrite(VEXT_ENABLE_PIN, LOW);
    sleep_for(10);
    const auto rred = analogRead(VBATT_READ_PIN);
    //digitalWrite(VEXT_ENABLE_PIN, HIGH);

    const float val = (map((long)(rred * (float)VBATT_PRECISION), 1050 * VBATT_PRECISION, 1375 * VBATT_PRECISION, 0, 100 * VBATT_PRECISION) * 0.01f / VBATT_PRECISION);

    __batt.m_level = (__batt.m_level * VBATT_SMOOTH + val) * 1.0f / (1.0f + VBATT_SMOOTH);

    return trunc ? (__batt.m_level > 1.0f ? 1.0f : (__batt.m_level < 0.0f ? 0.0f : __batt.m_level)) : __batt.m_level;
}