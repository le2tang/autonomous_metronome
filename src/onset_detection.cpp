#include "onset_detection.h"

OnsetDetection::~OnsetDetection() {
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
    num_bands_ = params.num_bands;
    channel_ = params.channel;

    onset_ = new float[num_bands];
    last_mag_sq_ = new float[num_bands];
    last_last_phase_ = new float[num_bands];
    last_phase_ = new float[num_bands];
    
    arm_rfft_fast_init_f16(&rfft_inst_, num_bands_);
}

float OnsetDetection::update(daisy::AudioHandle::InputBuffer in, size_t size) {
    float in_fft[size / 2 + 2];
    arm_rfft_fast_f16(&rfft_inst_, in[channel_], in_fft, 0);

    for (int idx = 0; idx < num_bands_; ++idx) {
        int idx_re = 2 * idx;
        int idx_im = 2 * idx + 1;
        
        float mag_sq = out_buf_[idx_re] * out_buf_[idx_re] + out_buf_[idx_im] * out_buf_[idx_im];
        float phase = atan2(out_buf_[idx_im], out_buf_[idx_re]);
    
        float phase_dev = phase - 2 * last_phase_[idx] + last_last_phase_[idx];
        onset_[idx] = mag_sq + last_mag_sq_[idx] - 2 * sqrt(mag_sq * last_mag_sq_[idx]) * cos(phase_dev);
    
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
