#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"

#include "memory.h"

void *operator new(size_t size) {
    ESP_LOGI("MEM", "alloc %dB", size);
    return pvPortMalloc(size);
}
void *operator new[](size_t size) {
    ESP_LOGI("MEM", "Alloc %dB", size);
    return pvPortMalloc(size);
}

void operator delete(void *ptr) {
    ESP_LOGI("MEM", "del %08X", (uint32_t)ptr);
    vPortFree(ptr);
}
void operator delete[](void *ptr) {
    ESP_LOGI("MEM", "Del %08X", (uint32_t)ptr);
    vPortFree(ptr);
}
