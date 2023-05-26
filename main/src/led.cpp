#include "led.h"

#include "driver/gpio.h"
#include "driver/pwm.h"

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"

Led::Led(gpio_num_t pin, int period, int channel)
    : pin_(pin), period_(period), channel_(channel) {
    int duty = 0;
    pwm_init(period_, &duty, channel_, &pin_);
    pwm_set_phase(channel_, 0);

    pwm_start();
}

void Led::set(float pwr) {
    int duty = pwr * period_;
    pwm_set_duty(channel_, duty);
    pwm_start();
}
