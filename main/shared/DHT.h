#pragma once

#include <driver/gpio.h>

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
