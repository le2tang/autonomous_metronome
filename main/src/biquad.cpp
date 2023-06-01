#include "biquad.h"

#include <math.h>

Biquad::Biquad(float b0, float b1, float b2, float a1, float a2)
    : b0_(b0), b1_(b1), b2_(b2), a1_(a1), a2_(a2), w1_(0), w2_(0) {}

float Biquad::filter(float x) {
    float w0 = x - a1_ * w1_ - a2_ * w2_;
    float y = b0_ * w0 + b1_ * w1_ + b2_ * w2_;

    w2_ = w1_;
    w1_ = w0;

    return y;
}

float Biquad::eval_freq_gain(float norm_freq) {
    double cos_freq = cos(norm_freq);
    double sin_freq = sin(norm_freq);

    double num_re = b0_ + b1_ * cos_freq +
                    b2_ * (cos_freq * cos_freq - sin_freq * sin_freq);
    double num_im = sin_freq * (b1_ + 2 * b2_ * cos_freq);

    double den_re =
        1 + a1_ * cos_freq + a2_ * (cos_freq * cos_freq - sin_freq * sin_freq);
    double den_im = sin_freq * (a1_ + 2 * a2_ * cos_freq);

    return sqrt((num_re * num_re + num_im * num_im) /
                (den_re * den_re + den_im * den_im));
}

Biquad *warped_tustin(const Biquad *analog, float sample_rate,
                      float warp_freq) {
    float K = 2 * M_PI * warp_freq / tan(M_PI * warp_freq / sample_rate);
    return tustin_(analog, K);
}

Biquad *tustin(const Biquad *analog, float sample_rate) {
    float K = 2 * sample_rate;
    return tustin_(analog, K);
}

Biquad *tustin_(const Biquad *analog, float K) {
    const float b0 = analog->b0();
    const float b1 = analog->b1();
    const float b2 = analog->b2();
    const float a1 = analog->a1();
    const float a2 = analog->a2();

    float den = K * K + a1 * K + a2;

    float bd0 = (b0 * K * K + b1 * K + b2) / den;
    float bd1 = (2 * b2 - 2 * b0 * K * K) / den;
    float bd2 = (b0 * K * K - b1 * K + b2) / den;
    float ad1 = (2 * a2 - 2 * K * K) / den;
    float ad2 = (K * K - a1 * K + a2) / den;

    return new Biquad(bd0, bd1, bd2, ad1, ad2);
}
