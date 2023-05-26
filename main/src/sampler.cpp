#include "sampler.h"

#include <cstddef>

#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include "freertos/task.h"

#include "driver/hw_timer.h"

#include "mcp3002.h"

void Sampler::init(const SamplerParams &params) {
    stream_buf_ = xStreamBufferCreate(
        params.buffer_size, params.buffer_trigger * params.sample_byte_size);

    sample_byte_size_ = params.sample_byte_size;

    hw_timer_init(callback_impl, NULL);
    hw_timer_set_load_data(params.sample_period_us);
    hw_timer_set_reload(true);
}

void Sampler::callback_impl(void *arg) {
    static_cast<Sampler *>(arg)->callback();
}

void Sampler::callback() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint16_t val = adc_.read();
    xStreamBufferSendFromISR(stream_buf_, (void *)&val, sample_byte_size_,
                             &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        taskYIELD();
    }
}

TickType_t Sampler::start_timer() {
    TickType_t curr_tick = xTaskGetTickCount();
    hw_timer_enable(true);

    return curr_tick;
}

void Sampler::stop_timer() { hw_timer_enable(false); }
