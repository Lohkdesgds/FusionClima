#pragma once

extern "C" {
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include <inttypes.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/queue.h"
    #include "driver/gpio.h"
    #include "driver/adc.h"
    #include "driver/dac.h"
}

#include <functional>
#include <initializer_list>
#include <mutex>
#include <utility>

#include "tools.h"

enum class ADC_num { ADC1, ADC2 };

struct gpio_config_custom {
    gpio_config_t c{};
    gpio_config_custom& set_mode(gpio_mode_t);
    gpio_config_custom& set_pull_up(bool);
    gpio_config_custom& set_pull_down(bool);
    gpio_config_custom& set_trigger(gpio_int_type_t);
    gpio_config_custom& set_pin(gpio_num_t);
};

struct gpio_event_mapping {
    struct _ev {
        gpio_num_t id;
        bool rise = false;
    };
    std::function<void(bool)> fmap[GPIO_NUM_MAX];
    bool last_state[GPIO_NUM_MAX]{};
    std::mutex fmap_m;
    QueueHandle_t que = xQueueCreate(30, sizeof(_ev));
    TaskHandle_t thr = nullptr;
    gpio_isr_handle_t gpio_reg;
};


/*
Important notes:
- custom_gpio_analog for ADC1 should be called before any readings
- ADC1 has fixed width for all pins
- ADC2 should not be used with WiFi enabled
*/

adc_channel_t custom_to_adc(gpio_num_t, ADC_num&);
dac_channel_t custom_to_dac(gpio_num_t);

bool custom_enable_gpio_functional(bool, BaseType_t = 0);

bool custom_gpio_setup(gpio_config_custom);
void custom_gpio_map_to_function(gpio_num_t, std::function<void(bool)>);

bool custom_gpio_setup_analogread(std::initializer_list<gpio_num_t>, adc_atten_t atten = ADC_ATTEN_DB_11, adc_bits_width_t = ADC_WIDTH_MAX /* valid on ADC1 once */);

bool custom_digital_read_cache(gpio_num_t);
bool custom_digital_read(gpio_num_t);
bool custom_digital_write(gpio_num_t, bool);

// read need custom_gpio_setup_analogread. adc_bits_width_t setting only available for adc2
int32_t custom_analog_read(gpio_num_t, adc_bits_width_t = static_cast<adc_bits_width_t>(SOC_ADC_MAX_BITWIDTH));
int32_t custom_analog_read(gpio_num_t);

// DAC will setup as we go
bool custom_analog_write(gpio_num_t, float);

void __custom_gpio_handler(void*);
void __async_gpio_handler(void*);