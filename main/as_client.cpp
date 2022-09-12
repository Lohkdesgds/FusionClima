#include "main_shared.h"

//void IRAM_ATTR lora_auto_receive(LoRa& arg);

void as_client()
{
    ESP_LOGI(CLITAG, "Started as client");

    ESP_LOGI(CLITAG, "Enabling LED for boot feedback");
    spin_on_fail(CLITAG, custom_gpio_setup(gpio_config_custom().set_pin(led_pin).set_mode(GPIO_MODE_OUTPUT)), "Cannot setup LED");
    custom_digital_write(led_pin, true);
    
    ESP_LOGI(CLITAG, "Preparing LoRa.");    
	lora = new LoRa( PIN_NUM_MOSI, PIN_NUM_MISO, PIN_NUM_CLK, PIN_NUM_CS, RESET_PIN, PIN_NUM_DIO, 10 );

    ESP_LOGI(CLITAG, "Working on button");
    spin_on_fail(CLITAG, custom_gpio_setup(gpio_config_custom().set_pin(GPIO_NUM_0).set_trigger(GPIO_INTR_ANYEDGE).set_mode(GPIO_MODE_INPUT)), "Cannot setup button #0");
    custom_gpio_map_to_function(GPIO_NUM_0,  [](bool a){ ESP_LOGI(CLITAG, "Button: %s", (a ? "DOWN" : "UP")); });
    custom_enable_gpio_functional(true);

    custom_digital_write(led_pin, false);
    ESP_LOGI(CLITAG, "Ready.");


    while(1){
        delay(20);
        if (lora->getDataReceived()) {
            custom_digital_write(led_pin, true);
            std::string buf;
            if (lora->handleDataReceived(buf)) ESP_LOGI(CLITAG, "Got from LoRa: %s", buf.c_str());
            else ESP_LOGI(CLITAG, "Failed to get data from LoRa");
            delay(10);
            custom_digital_write(led_pin, false);
        }
    }
}