#include "biquad.h"

Biquad::Biquad(float b0, float b1, float b2, float a1, float a2)
    : b0_(b0), b1_(b1), b2_(b2), a1_(a1), a2_(a2), w1_(0), w2_(0) {}

float Biquad::filter(float x) {
    float w0 = x - a1_ * w1_ - a2_ * w2_;
    float y = b0_ * w0 + b1_ * w1_ + b2_ * w2_;

    w2_ = w1_;
    w1_ = w0;

    return y;
}