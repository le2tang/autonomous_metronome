#include "sampler.h"

#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include "freertos/task.h"

#include "driver/hw_timer.h"

#include "mcp3002.h"

void Sampler::init(const SamplerParams &params) {
    stream_buf_ = xStreamBufferCreate(
        params.buffer_size, params.buffer_trigger * params.sample_byte_size);

    sample_byte_size_ = params.sample_byte_size;

    hw_timer_init(timer_callback_, NULL);
    hw_timer_set_load_data(timer_period_);
    hw_timer_set_reload(true);
}

TickType_t Sampler::start_timer() {
    TickType_t curr_tick = xTaskGetTickCount();
    hw_timer_enable(true);

    return curr_tick;
}

void Sampler::stop_timer() { hw_timer_enable(false); }

void Sampler::timer_callback(void *arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint16_t val = adc_.read();
    xStreamBufferSendFromISR(stream_buf_, (void *)&val, sample_byte_size_,
                             &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        taskYIELD();
    }
}
