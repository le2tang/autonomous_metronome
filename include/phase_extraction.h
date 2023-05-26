#pragma once

#include "daisy.h"
#include "util.h"

struct PhaseExtractionParams {
    int num_samples;
    float sample_rate;
};

class PhaseExtraction {
    void init(const PhaseExtractionParams &params, const Buffer &in);

    float update(float tempo_freq);

private:
    int num_samples_;
    float sample_rate_;
    const Buffer *in_buf_;

    int start_time_sample_;
};
