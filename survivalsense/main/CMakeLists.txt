idf_component_register(
    SRCS
        "main.c"
        "wifi_manager.c"
        "firebase_upload.c"
        "dht11.c"
    INCLUDE_DIRS "."
    REQUIRES
        nvs_flash
        esp_wifi
        esp_event
        esp_http_client
        esp_netif
        esp_driver_gpio
        esp_timer
)