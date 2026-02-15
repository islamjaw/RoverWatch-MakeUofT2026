#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "shared_data.h"
#include "wifi_manager.h"
#include "firebase_upload.h"

static const char *TAG = "Main";

survival_data_t g_data = {0};

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
    xTaskCreate(upload_task, "upload", 4096, NULL, 3, NULL);
    ESP_LOGI(TAG, "All tasks started");
}