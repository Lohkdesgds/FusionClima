#include "shared/tools.h"
#include "shared/buttons.h"
#include "shared/display.h"

const char TAG[] = "MAINCLI";

void as_client()
{
    ESP_LOGI(TAG, "Started as client");

    spin_on_fail(TAG, custom_gpio_setup(gpio_config_custom().set_pin(GPIO_NUM_0).set_trigger(GPIO_INTR_ANYEDGE).set_mode(GPIO_MODE_INPUT)), "Cannot setup button #0");

    custom_gpio_map_to_function(GPIO_NUM_0, 
        [](bool a){ 
            ESP_LOGI(TAG, "Button: %s", (a ? "DOWN" : "UP"));
        }
    );

    custom_enable_gpio_functional(true);

    while(1) delay(200);
}