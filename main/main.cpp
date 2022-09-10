extern "C" {
    void app_main(void);
}

extern void as_client();
extern void as_host();

#include "shared/tools.h"
#include "shared/buttons.h"
#include "shared/DHT.h"

const char TAG[] = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting app in 1 second...");

    delay(1000);

    spin_on_fail(TAG, custom_gpio_setup(gpio_config_custom().set_pin(GPIO_NUM_17).set_pull_up(true).set_mode(GPIO_MODE_INPUT)), "Cannot setup selector button");

    if (custom_digital_read(GPIO_NUM_17)) as_client();
    else as_host();

    spin_on_fail(TAG, false, "Returned function that was not meant to be returned: main");
}
