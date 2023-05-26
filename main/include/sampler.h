#pragma once

#include <cstddef>

#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include "freertos/task.h"

#include "mcp3002.h"

struct SamplerParams {
    int buffer_size;
    int buffer_trigger;
    int sample_byte_size;
    int sample_period_us;
};

class Sampler {
  public:
    Sampler() {}
    ~Sampler() {}

    void init(const SamplerParams &params);

    static void callback_impl(void *arg);
    void callback();

    TickType_t start_timer();
    void stop_timer();

    inline StreamBufferHandle_t get_stream_buffer() { return stream_buf_; }

  private:
    StreamBufferHandle_t stream_buf_;
    MCP3002 adc_;

    int sample_byte_size_;
};
