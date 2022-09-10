#pragma once

#include <stdio.h>
#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include "tools.h"

constexpr uint8_t DHT11{11};  /**< DHT TYPE 11 */
constexpr uint8_t DHT12{12};  /**< DHY TYPE 12 */
constexpr uint8_t DHT21{21};  /**< DHT TYPE 21 */
constexpr uint8_t DHT22{22};  /**< DHT TYPE 22 */
constexpr uint8_t AM2301{21}; /**< AM2301 */

class DHT {
public:
  DHT(gpio_num_t pin, uint8_t type, uint8_t count = 6);
  void begin(uint8_t usec = 55);
  float readTemperature(bool S = false, bool force = false);
  float convertCtoF(float);
  float convertFtoC(float);
  float computeHeatIndex(bool isFahrenheit = true);
  float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit = true);
  float readHumidity(bool force = false);
  bool read(bool force = false);

private:
  uint8_t data[5];
  gpio_num_t _pin;
  uint8_t _type;
  uint32_t _lastreadtime;//, _maxcycles; // assume 1 ms
  bool _lastresult;
  uint8_t pullTime;

  uint32_t expectPulse(bool level);
};