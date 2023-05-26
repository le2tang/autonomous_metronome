#include "rfft.h"

struct OnsetDetectionParams {
    int num_samples;
    int num_bands;
};

#pragma once

class OnsetDetection {
    ~OnsetDetection();

    void init(const OnsetDetectionParams &params);

    float update(float *in);

  private:
    int num_samples_;
    int num_bands_;

    fft::RFFTParams rfft_params_;
    fft::RFFT rfft_inst_;

    float *onset_;
    float *last_mag_sq_;
    float *last_last_phase_;
    float *last_phase_;
};
