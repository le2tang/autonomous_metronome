#pragma once

#include "util.h"

struct PhaseExtractionParams {
    int num_samples;
    float sample_rate;
};

#ifdef __cplusplus
extern "C" {
#endif

class PhaseExtraction {
  public:
    PhaseExtraction() {}
    ~PhaseExtraction() {}

    void init(const PhaseExtractionParams &params, const Buffer &in);

    float update(float tempo_freq);
    void reset_start_time();

  private:
    int num_samples_;
    float sample_rate_;
    const Buffer *in_buf_;

    int start_time_sample_;
};

#ifdef __cplusplus
}
#endif
