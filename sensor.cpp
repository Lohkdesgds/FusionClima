#include "sensor.h"

void Sensor::_loop()
{
  float tmp = m_dht.readTemperature();
  float hum = m_dht.readHumidity();

  if (isnan(tmp) || isnan(hum)) {
    if (m_err_hdlr) m_err_hdlr(e_sensor_errors::E_NAN);
    return;
  }
  
  m_hum = (m_hum + hum * f_sensor_prop) * 1.0f / (1.0f + f_sensor_prop);
  m_tmp = (m_tmp + tmp * f_sensor_prop) * 1.0f / (1.0f + f_sensor_prop);
  m_hum_d = hum - m_hum;
  m_tmp_d = tmp - m_tmp;
  m_heat = m_dht.computeHeatIndex(m_tmp, m_hum, false);
  
  std::this_thread::sleep_until(m_last);
  m_last = std::chrono::system_clock::now() + std::chrono::seconds(SENSOR_TIME_DELTA);
}

Sensor::Sensor(std::function<void(e_sensor_errors)> hdlr)
  : Async(false), m_dht(SENSOR_PIN, SENSOR_TYPE), m_last(std::chrono::system_clock::now() + std::chrono::seconds(SENSOR_TIME_DELTA)), m_err_hdlr(hdlr)
{
  m_dht.begin();
  delayed_launch();  
}


float Sensor::get_temperature() const
{
  return m_tmp;
}

float Sensor::get_humidity() const
{
  return m_hum;
}

float Sensor::get_heatindex() const
{
  return m_heat;
}

float Sensor::get_temperature_last_delta() const
{
  return m_tmp_d;
}

float Sensor::get_humidity_last_delta() const
{
  return m_hum_d;
}
