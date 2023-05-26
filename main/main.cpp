/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "main.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/stream_buffer.h"

#include "driver/gpio.h"
#include "driver/hw_timer.h"
#include "driver/pwm.h"

#include "esp_log.h"
#include "esp_system.h"

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"

#ifdef __cplusplus
extern "C"
{
#endif

    const char *TAG = "MCP3002_SPI";

    StreamBufferHandle_t stream_buf = NULL;

    void timer_callback(void *arg)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        static uint8_t state = 0;

        gpio_set_level(GPIO_NUM_16, state % 2);
        ++state;

        uint16_t val = mcp3002_read();
        xStreamBufferSendFromISR(stream_buf, (void *)&val, 2, &xHigherPriorityTaskWoken);

        if (xHigherPriorityTaskWoken)
        {
            taskYIELD();
        }
    }

    void setup_timer()
    {
        stream_buf = xStreamBufferCreate(128, 128);

        hw_timer_init(timer_callback, NULL);
        hw_timer_alarm_us(100, true);
    }

    void setup_spi()
    {
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

    void setup_gpio()
    {
        gpio_config_t io_conf;
        // disable interrupt
        io_conf.intr_type = GPIO_INTR_DISABLE;
        // set as output mode
        io_conf.mode = GPIO_MODE_OUTPUT;
        // bit mask of the pins that you want to set,e.g.GPIO15/16
        io_conf.pin_bit_mask = (1ULL << 16);
        // disable pull-down mode
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        // disable pull-up mode
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        // configure GPIO with the given settings
        gpio_config(&io_conf);
    }

    uint16_t mcp3002_read()
    {
        uint8_t cmd = 0b01101000;
        uint16_t val = 0;

        // Set CS LOW
        gpio_set_level(GPIO_NUM_15, 0);

        for (int idx = 0; idx < 5; ++idx)
        {
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

        for (int idx = 0; idx < 11; ++idx)
        {
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

    void mcp3002_read_task(void *arg)
    {
        size_t xReceivedBytes;
        uint16_t buf[64];

        uint32_t iter = 0;

        while (1)
        {
            xReceivedBytes = xStreamBufferReceive(stream_buf, buf, 128, 100 / portTICK_RATE_MS);
            if (xReceivedBytes)
            {
                float avg = 0;
                for (int idx = 0; idx < 64; ++idx)
                {
                    avg += buf[idx];
                }
                avg /= 64;
                ESP_LOGI(TAG, "%d[%d]: %d\n", iter, xReceivedBytes, (int)avg);
            }

            ++iter;
            // vTaskDelay(100 / portTICK_RATE_MS);
        }
    }

    void app_main()
    {
        setup_timer();
        setup_spi();
        setup_gpio();

        xTaskCreate(mcp3002_read_task, "mcp3002_read_task", 2048, NULL, 2, NULL);
    }

#ifdef __cplusplus
}
#endif
