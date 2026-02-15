#pragma once

typedef struct {
    float temperature;
    float humidity;
    int   status;  // 0 = OK, -1 = error
} dht11_reading_t;

void dht11_init(int gpio_pin);
dht11_reading_t dht11_read(void);