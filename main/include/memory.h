#pragma once

#include "esp_system.h"

#ifdef __cplusplus
extern "C" {
#endif

void *operator new(size_t size);
void *operator new[](size_t size);

void operator delete(void *ptr);
void operator delete[](void *ptr);

#ifdef __cplusplus
}
#endif
