#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"

#include "memory.h"

void *operator new(size_t size) { return pvPortMalloc(size); }
void *operator new[](size_t size) { return pvPortMalloc(size); }

void operator delete(void *ptr) { vPortFree(ptr); }
void operator delete[](void *ptr) { vPortFree(ptr); }
