#pragma once

#include <driver/gpio.h>

// Based on: https://github.com/iotechbugs/esp32/blob/master/DHT22-cpp/main/DHT.h (thank you)

class DHT {
	gpio_num_t DHTgpio;
	float humidity = 0.0f;
	float temperature = 0.0f;

	int getSignalLevel(int, bool);
public:
	DHT(gpio_num_t);
	
	bool update();
	float getHumidity() const;
	float getTemperature() const;
};
