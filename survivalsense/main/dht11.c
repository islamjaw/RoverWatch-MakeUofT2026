#include "dht11.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "DHT11";
static int dht_gpio = 4;

void dht11_init(int gpio_pin) {
    dht_gpio = gpio_pin;
    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 1);
    vTaskDelay(pdMS_TO_TICKS(1000)); // let sensor stabilize
    ESP_LOGI(TAG, "DHT11 initialized on GPIO %d", gpio_pin);
}

static int wait_for_level(int level, int timeout_us) {
    int elapsed = 0;
    while (gpio_get_level(dht_gpio) != level) {
        if (elapsed >= timeout_us) return -1;
        esp_rom_delay_us(1);
        elapsed++;
    }
    return elapsed;
}

dht11_reading_t dht11_read(void) {
    dht11_reading_t result = {0, 0, -1};
    uint8_t data[5] = {0};

    // Send start signal
    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 0);
    vTaskDelay(pdMS_TO_TICKS(20));  // pull low 20ms
    gpio_set_level(dht_gpio, 1);
    esp_rom_delay_us(30);

    // Switch to input
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);

    // Wait for DHT11 response
    if (wait_for_level(0, 100) < 0) {
        ESP_LOGE(TAG, "No response from DHT11 (waiting for LOW)");
        return result;
    }
    if (wait_for_level(1, 100) < 0) {
        ESP_LOGE(TAG, "No response from DHT11 (waiting for HIGH)");
        return result;
    }
    if (wait_for_level(0, 100) < 0) {
        ESP_LOGE(TAG, "No response from DHT11 (waiting for data start)");
        return result;
    }

    // Read 40 bits
    for (int i = 0; i < 40; i++) {
        if (wait_for_level(1, 100) < 0) return result;

        int high_time = wait_for_level(0, 100);
        if (high_time < 0) return result;

        data[i / 8] <<= 1;
        if (high_time > 40) {
            data[i / 8] |= 1;  // long pulse = 1, short = 0
        }
    }

    // Verify checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        ESP_LOGE(TAG, "Checksum failed: %d != %d", checksum, data[4]);
        return result;
    }

    result.humidity    = data[0] + data[1] * 0.1f;
    result.temperature = data[2] + data[3] * 0.1f;
    result.status      = 0;

    ESP_LOGI(TAG, "Temp: %.1f°C  Humidity: %.1f%%",
             result.temperature, result.humidity);
    return result;
}