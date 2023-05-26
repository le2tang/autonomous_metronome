#include "daisy.h"

struct OnsetDetectionParams {
    int num_bands;
    int channel;
};

#pragma once

class OnsetDetection {
    ~OnsetDetection();
    
    void init(const OnsetDetectionParams &params);

    float update(daisy::AudioHandle::InputBuffer in, size_t size);

private:
    int num_bands_;
    int channel_;

    arm_rfft_fast_instance_f16 rfft_inst_;

    float *onset_;
    float *last_mag_sq_;
    float *last_last_phase_;
    float *last_phase_;
};
