#include "shared/tools.h"
#include "shared/buttons.h"
#include "shared/DHT.h"

const char TAG[] = "MAINHST";

void as_host()
{
    ESP_LOGI(TAG, "Started as host");

    DHT dht(GPIO_NUM_32);

    while(1) {
        dht.update();
        ESP_LOGI(TAG, "Temp: %.1f; Hum: %.1f", dht.getTemperature(), dht.getHumidity());
        delay(2000);
    }
}