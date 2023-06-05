#pragma once

#include "esp_system.h"

#include "rfft.h"

struct OnsetDetectionParams {
    int num_samples;
    int num_bands;
};

#ifdef __cplusplus
extern "C" {
#endif

class OnsetDetection {
  public:
    OnsetDetection() {}
    ~OnsetDetection();

    void init(const OnsetDetectionParams &params);

    void load_input(uint16_t *in);
    float update();

  private:
    int num_samples_;
    int num_bands_;

    float *input_;
    float *freq_weight_;

    fft::RFFTParams rfft_params_;
    fft::RFFT rfft_inst_;

    float *onset_;
    float *last_mag_sq_;
    float *last_last_phase_;
    float *last_phase_;
};

#ifdef __cplusplus
}
#endif
