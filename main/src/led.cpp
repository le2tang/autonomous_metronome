#include "led.h"

#include "driver/gpio.h"
#include "driver/pwm.h"

#include "esp_system.h"

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"

void Led::init() {
    pin_ = GPIO_NUM_16;
    period_ = 1000;
    channel_ = 0;

    uint32_t duty = 0;
    pwm_init(period_, &duty, 1, &pin_);
    pwm_set_phase(channel_, 0);

    pwm_start();
}

void Led::set(float pwr) {
    uint32_t duty = pwr * period_;
    pwm_set_duty(channel_, duty);
    pwm_start();
}
