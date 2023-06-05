#include "sampler.h"

#include <cstddef>

#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "driver/hw_timer.h"

void Sampler::init(const SamplerParams &params) {
    trigger_byte_size_ = params.buffer_trigger * params.sample_byte_size;
    sample_byte_size_ = params.sample_byte_size;

    stream_buf_ = xStreamBufferCreate(params.buffer_size, trigger_byte_size_);

    adc_.init();

    hw_timer_init(callback_impl, this);
    hw_timer_set_reload(true);
    hw_timer_set_clkdiv(TIMER_CLKDIV_1);
    hw_timer_set_intr_type(TIMER_EDGE_INT);
    hw_timer_set_load_data(((TIMER_BASE_CLK >> hw_timer_get_clkdiv()) / 1E6) *
                           params.sample_period_us);
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
