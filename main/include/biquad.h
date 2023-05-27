#pragma once

#ifdef __cplusplus
extern "C" {
#endif

class Biquad {
  public:
    Biquad(float b0, float b1, float b2, float a1, float a2);
    ~Biquad() {}

    inline float b0() const { return b0_; }
    inline float b1() const { return b1_; }
    inline float b2() const { return b2_; }
    inline float a1() const { return a1_; }
    inline float a2() const { return a2_; }

    float filter(float x);

  private:
    const float b0_, b1_, b2_;
    const float a1_, a2_;
    float w1_, w2_;
};

#ifdef __cplusplus
}
#endif
