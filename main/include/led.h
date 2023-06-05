#pragma once

#include "esp_system.h"

#ifdef __cplusplus
extern "C" {
#endif

class PWMLed {
  public:
    PWMLed() {}
    ~PWMLed() {}

    void init(int pin);
    void set(float pwr);

  private:
    uint32_t pin_;
    uint32_t period_;
    uint32_t channel_;
};

class Led {
  public:
    Led() {}
    ~Led() {}
    void init(int pin);
    void set(bool state);

  private:
    int pin_;
};

#ifdef __cplusplus
}
#endif
