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

float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit)
{
  float hi;

  const auto convertCtoF = [](float c) -> float { return c * 1.8 + 32; };
  const auto convertFtoC = [](float f) -> float { return (f - 32) * 0.55555; };
  
  if (!isFahrenheit)
    temperature = convertCtoF(temperature);

  hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) +
              (percentHumidity * 0.094));

  if (hi > 79) {
    hi = -42.379 + 2.04901523 * temperature + 10.14333127 * percentHumidity +
         -0.22475541 * temperature * percentHumidity +
         -0.00683783 * pow(temperature, 2) +
         -0.05481717 * pow(percentHumidity, 2) +
         0.00122874 * pow(temperature, 2) * percentHumidity +
         0.00085282 * temperature * pow(percentHumidity, 2) +
         -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

    if ((percentHumidity < 13) && (temperature >= 80.0) &&
        (temperature <= 112.0))
      hi -= ((13.0 - percentHumidity) * 0.25) *
            sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

    else if ((percentHumidity > 85.0) && (temperature >= 80.0) &&
             (temperature <= 87.0))
      hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
  }

  return isFahrenheit ? hi : convertFtoC(hi);
}
