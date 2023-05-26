#pragma once

#include "freertos/stream_buffer.h"

struct SamplerParams {
    int buffer_size;
    int buffer_trigger;
    int sample_byte_size;
};

class Sampler {
  public:
    void init(const SamplerParams &params);

    TickType_t start_timer();
    void stop_timer();

    StreamBufferHandle_t get_stream_buffer() { return stream_buf_; }

  private:
    void timer_callback(void *arg);

    StreamBufferHandle_t stream_buf_;
    MCP3002 adc_;

    int sample_byte_size_;
};
