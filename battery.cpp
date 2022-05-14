#include "battery.h"

void setup_battery()
{
  pinMode(VBATT_READ_PIN, OPEN_DRAIN);
  pinMode(VEXT_ENABLE_PIN, OUTPUT);
  digitalWrite(VEXT_ENABLE_PIN, LOW);
}

float read_battery_perc(bool limit_range)
{
  //digitalWrite(VEXT_ENABLE_PIN, LOW);
  delay(8);  
  float sum = 0.0f;
  for(unsigned p = 0; p < VBATT_SMOOTH_DEF; ++p) {
    delay(2);
    sum += analogRead(VBATT_READ_PIN);
  }
  //digitalWrite(VEXT_ENABLE_PIN, HIGH);  

  sum /= VBATT_SMOOTH_DEF;
  
  float _val = (map((long)(sum * (float)VBATT_PRECISION), 1600 * VBATT_PRECISION, 2020 * VBATT_PRECISION, 0, 100 * VBATT_PRECISION) * 1.0f / VBATT_PRECISION);

  if (_battinf.last_read > -50.0f) {
    if (_val > 100.0f) {
      _battinf.tendency = 4;
    }
    else if (_battinf.last_read < _val && (_battinf.tendency < VBATT_LIMIT_TENDENCY)) _battinf.tendency++;
    else if (_battinf.last_read > _val && (_battinf.tendency > -VBATT_LIMIT_TENDENCY)) _battinf.tendency--;
    /*if (_val > 100.0f) {
      _battery_on_charge = true;
    }
    else {
      if (_battery_last_read + COEF_F_DISCHARGE_TEST < _val) _battery_on_charge = true;
      else if (_val + COEF_F_DISCHARGE_TEST < _battery_last_read) _battery_on_charge = false;
    }*/
  }
  _battinf.last_read = (_battinf.last_read * VBATT_AVG_ADPT + _val) * 1.0f / (VBATT_AVG_ADPT + 1.0f);
  
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

bool get_battery_charging()
{
  return _battinf.tendency > 0;
}
