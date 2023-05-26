#include "iir_filter.h"

#include <math.h>

#include "biquad.h"

IIRFilter::IIRFilter(int num_stages)
    : num_stages_(num_stages), stages_(new Biquad *[num_stages]) {}

IIRFilter::~IIRFilter() {
    if (stages_) {
        delete[] stages_;
    }
}

void IIRFilter::set_stage(int idx, Biquad *stage) { stages_[idx] = stage; }

Biquad *IIRFilter::get_stage(int idx) { return stages_[idx]; }

const Biquad *IIRFilter::get_stage(int idx) const { return stages_[idx]; }

float IIRFilter::filter(float x) {
    float y = x;
    for (int idx = 0; idx < num_stages_; ++idx) {
        y = stages_[idx]->filter(y);
    }
    return y;
}

Biquad *warped_tustin(Biquad *analog, float sample_rate, float warp_freq) {
    float K = 2 * M_PI * warp_freq / tan(M_PI * warp_freq / sample_rate);
    return _tustin(analog, K);
}

Biquad *tustin(Biquad *analog, float sample_rate) {
    float K = 2 * sample_rate;
    return _tustin(analog, K);
}

Biquad *_tustin(Biquad *analog, float K) {
    float b0 = analog->b0;
    float b1 = analog->b1;
    float b2 = analog->b2;
    float a1 = analog->a1;
    float a2 = analog->a2;

    float den = K * K + a1 * K + a2;

    float bd0 = (b0 * K * K + b1 * K + b2) / den;
    float bd1 = (2 * b2 - 2 * b0 * K * K) / den;
    float bd2 = (b0 * K * K - b1 * K + b2) / den;
    float ad1 = (2 * a2 - 2 * a0 * K * K) / den;
    float ad2 = (K * K - a1 * K + a2) / den;

    return new Biquad(bd0, bd1, bd2, ad1, ad2);
}