#include "onset_detection.h"

#include <cstring>
#include <math.h>

#include "esp_log.h"
#include "esp_system.h"

#include "memory.h"

OnsetDetection::~OnsetDetection() {
    if (input_) {
        delete[] input_;
    }
    if (onset_) {
        delete[] onset_;
    }
    if (last_mag_sq_) {
        delete[] last_mag_sq_;
    }
    if (last_last_phase_) {
        delete[] last_last_phase_;
    }
    if (last_phase_) {
        delete[] last_phase_;
    }
}

void OnsetDetection::init(const OnsetDetectionParams &params) {
    num_samples_ = params.num_samples;
    num_bands_ = params.num_bands;

    input_ = new float[params.num_samples + 2];
    memset(input_, 0, (params.num_samples + 2) * sizeof(float));

    rfft_inst_.twiddle_init(num_samples_);
    rfft_params_.init(num_samples_);

    onset_ = new float[num_bands_];
    last_mag_sq_ = new float[num_bands_];
    last_last_phase_ = new float[num_bands_];
    last_phase_ = new float[num_bands_];

    memset(onset_, 0, num_bands_ * sizeof(float));
    memset(last_mag_sq_, 0, num_bands_ * sizeof(float));
    memset(last_last_phase_, 0, num_bands_ * sizeof(float));
    memset(last_phase_, 0, num_bands_ * sizeof(float));
}

void OnsetDetection::load_input(uint16_t *in) {
    for (int idx = 0; idx < num_samples_; ++idx) {
        input_[idx] = (float)in[idx] / (float)(1ULL << 16);
    }
    input_[num_samples_] = 0;
    input_[num_samples_ + 1] = 0;
}

float OnsetDetection::update() {
    rfft_inst_.rfft(input_, rfft_params_);

    for (int idx = 1; idx < num_bands_; ++idx) {
        int idx_re = 2 * idx;
        int idx_im = 2 * idx + 1;

        float mag_sq =
            input_[idx_re] * input_[idx_re] + input_[idx_im] * input_[idx_im];
        float phase = atan2(input_[idx_im], input_[idx_re]);

        if (phase - last_phase_[idx] >= M_PI) {
            phase -= 2 * M_PI;
        } else if (last_phase_[idx] - phase >= M_PI) {
            phase += 2 * M_PI;
        }

        float phase_dev = phase - 2 * last_phase_[idx] + last_last_phase_[idx];
        onset_[idx] =
            sqrt(mag_sq + last_mag_sq_[idx] -
                 2 * sqrt(mag_sq * last_mag_sq_[idx]) * cos(phase_dev));

        last_mag_sq_[idx] = mag_sq;
        last_last_phase_[idx] = last_phase_[idx];
        last_phase_[idx] = phase;
    }

    float onset_sum = 0;
    for (int idx = 0; idx < num_bands_; ++idx) {
        onset_sum += onset_[idx];
    }

    return onset_sum;
}
