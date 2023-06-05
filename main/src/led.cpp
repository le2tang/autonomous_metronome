#include "led.h"

#include "driver/gpio.h"
#include "driver/pwm.h"

#include "esp_system.h"

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"

void Led::init() {
    pin_ = 5;
    period_ = 1000;
    channel_ = 0;

    uint32_t duty = 0;
    pwm_init(period_, &duty, 1, &pin_);
    pwm_set_phase(channel_, 0);

    pwm_start();

    // gpio_config_t io_conf;
    // // disable interrupt
    // io_conf.intr_type = GPIO_INTR_DISABLE;
    // // set as output mode
    // io_conf.mode = GPIO_MODE_OUTPUT;
    // // bit mask of the pins that you want to set,e.g.GPIO15/16
    // io_conf.pin_bit_mask = (1ULL << 5);
    // // disable pull-down mode
    // io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // // disable pull-up mode
    // io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    // // configure GPIO with the given settings
    // gpio_config(&io_conf);
}

void Led::set(float pwr) {
    uint32_t duty = pwr * period_;
    pwm_set_duty(channel_, duty);
    pwm_start();

    // if (pwr > 0.9) {
    //     gpio_set_level(GPIO_NUM_5, 1);
    // } else {
    //     gpio_set_level(GPIO_NUM_5, 0);
    // }
}
