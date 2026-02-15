#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "shared_data.h"
#include "wifi_manager.h"
#include "firebase_upload.h"
#include "dht11.h"

static const char *TAG = "Main";

survival_data_t g_data = {0};

#define DHT11_GPIO 4

void sensor_task(void *pvParameters) {
    dht11_init(DHT11_GPIO);
    while (1) {
        dht11_reading_t reading = dht11_read();
        if (reading.status == 0) {
            g_data.temperature  = reading.temperature;
            g_data.humidity     = reading.humidity;
            g_data.upload_ready = true;
        } else {
            ESP_LOGW(TAG, "DHT11 read failed, retrying...");
        }
        vTaskDelay(pdMS_TO_TICKS(3000)); // read every 3 seconds
    }
}

void upload_task(void *pvParameters) {
    firebase_init();
    while (1) {
        if (g_data.wifi_connected && g_data.upload_ready) {
            firebase_upload(&g_data);
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "SurvivalSense booting...");
    wifi_init();
    wifi_wait_connected(15);
    xTaskCreate(sensor_task, "dht11",  2048, NULL, 5, NULL);
    xTaskCreate(upload_task, "upload", 4096, NULL, 3, NULL);
    ESP_LOGI(TAG, "All tasks started");
}