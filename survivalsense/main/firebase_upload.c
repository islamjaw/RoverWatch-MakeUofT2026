#include "firebase_upload.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "Firebase";

// #define FIREBASE_URL \
//   "https://survivalsense-a48fe-default-rtdb.firebaseio.com/"

#define FIREBASE_URL "https://survivalsense-a48fe-default-rtdb.firebaseio.com/sensor_data.json"

void firebase_init(void) {
    ESP_LOGI(TAG, "Firebase ready");
}

void firebase_upload(survival_data_t *data) {
    char payload[512];
    snprintf(payload, sizeof(payload),
        "{\"temperature\":%.1f,\"humidity\":%.1f,"
        "\"air_quality\":%d,\"heart_rate\":%.1f,"
        "\"food_result\":\"%s\",\"timestamp\":%ld}",
        data->temperature, data->humidity,
        data->air_quality, data->heart_rate,
        data->food_result, data->timestamp
    );
    esp_http_client_config_t config = {
        .url    = FIREBASE_URL,
        .method = HTTP_METHOD_POST,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, payload, strlen(payload));
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Upload successful");
    } else {
        ESP_LOGE(TAG, "Upload failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}