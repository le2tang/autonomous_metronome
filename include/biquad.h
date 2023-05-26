#pragma once

class Biquad {
public:
    Biquad(float b0, float b1, float b2, float a1, float a2);

    float filter(float x);

private:
    const float b0_, b1_, b2_;
    const float a1_, a2_;
    float w1_, w2_;
};