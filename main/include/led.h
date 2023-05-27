#pragma once

#include "esp_system.h"

#ifdef __cplusplus
extern "C" {
#endif

class Led {
  public:
    Led() {}
    ~Led() {}

    void init();
    void set(float pwr);

  private:
    uint32_t pin_;
    uint32_t period_;
    uint32_t channel_;
};

#ifdef __cplusplus
}
#endif
