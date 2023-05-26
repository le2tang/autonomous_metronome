#pragma once

#include "driver/gpio.h"

class Led {
  public:
    Led(gpio_num_t pin, int period, int channel);
    void set(float pwr);

  private:
    gpio_num_t pin_;
    int period_;
    int channel_;
};
