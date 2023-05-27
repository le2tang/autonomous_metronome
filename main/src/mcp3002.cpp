#include "mcp3002.h"

#include "driver/gpio.h"

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"

void MCP3002::init() {
    gpio_config_t io_conf;
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = (1ULL << 13) | (1ULL << 14) | (1ULL << 15);
    // disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    // configure GPIO with the given settings
    gpio_config(&io_conf);

    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    // bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = (1ULL << 12);
    // disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    // disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
}

uint16_t MCP3002::read() {
    uint8_t cmd = 0b01101000;
    uint16_t val = 0;

    // Set CS LOW
    gpio_set_level(GPIO_NUM_15, 0);

    for (int idx = 0; idx < 5; ++idx) {
        // Set MOSI
        if (cmd & 0x80)
            gpio_set_level(GPIO_NUM_13, 1);
        else
            gpio_set_level(GPIO_NUM_13, 0);
        cmd <<= 1;

        // Set CLK HIGH
        gpio_set_level(GPIO_NUM_14, 1);

        // Set CLK LOW
        gpio_set_level(GPIO_NUM_14, 0);
    }

    for (int idx = 0; idx < 11; ++idx) {
        val |= gpio_get_level(GPIO_NUM_12);
        val <<= 1;

        // Set CLK HIGH
        gpio_set_level(GPIO_NUM_14, 1);

        // Set CLK LOW
        gpio_set_level(GPIO_NUM_14, 0);
    }

    // Set CS HIGH
    gpio_set_level(GPIO_NUM_15, 1);

    return val;
}