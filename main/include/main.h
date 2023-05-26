#pragma once

#include "esp_system.h"

#ifdef __cplusplus
extern "C" {
#endif

void timer_callback(void *arg);

void setup_timer();
void setup_spi();
void setup_gpio();

uint16_t mcp3002_read();

void mcp3002_read_task(void *arg);

void app_main();

#ifdef __cplusplus
}
#endif
