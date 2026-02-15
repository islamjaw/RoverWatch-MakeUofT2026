#pragma once
#include <stdbool.h>

void wifi_init(void);
bool wifi_is_connected(void);
void wifi_wait_connected(int timeout_seconds);