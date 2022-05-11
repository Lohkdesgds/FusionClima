#pragma once

#include <Arduino.h>
#include "DHT.h"
#include "async.h"
#include <chrono>
#include <functional>
#include <mutex>

#ifndef SENSOR_PIN
#define SENSOR_PIN 32
#endif
#ifndef SENSOR_TYPE
#define SENSOR_TYPE DHT11 // testbench
#endif

#define SENSOR_TIME_DELTA 5 // sec

constexpr float f_sensor_prop = 2.0f;

enum class e_sensor_errors { E_NAN };

// async
class Sensor : public Async {
  DHT m_dht;
  float m_hum = -1.0f, m_hum_d = 0.0f; // perc
  float m_tmp = -1.0f, m_tmp_d = 0.0f; // celsius
  float m_heat = 0.0f;
  decltype(std::chrono::system_clock::now()) m_last;
  std::function<void(e_sensor_errors)> m_err_hdlr;

  void _loop();
public:
  Sensor(std::function<void(e_sensor_errors)>);

  float get_temperature() const;
  float get_humidity() const;
  float get_heatindex() const;

  // curr - stored now
  float get_temperature_last_delta() const;
  // curr - stored now
  float get_humidity_last_delta() const;
};
