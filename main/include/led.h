#pragma once

#include "driver/gpio.h"

class Led {
  public:
    Led() {}
    ~Led() {}

    void init();
    void set(float pwr);

  private:
    gpio_num_t pin_;
    int period_;
    int channel_;
};
