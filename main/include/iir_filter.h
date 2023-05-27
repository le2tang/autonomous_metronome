#pragma once

#include "biquad.h"

#ifdef __cplusplus
extern "C" {
#endif

class IIRFilter {
  public:
    IIRFilter(int num_stages);
    ~IIRFilter();

    void set_stage(int idx, Biquad *stage);
    Biquad *get_stage(int idx);
    const Biquad *get_stage(int idx) const;

    inline int num_stages() const { return num_stages_; };

    float filter(float x);

  protected:
    const int num_stages_;
    Biquad **stages_;
};

Biquad *warped_tustin(Biquad *analog, float sample_rate, float warp_freq);
Biquad *tustin(Biquad *analog, float sample_rate);
Biquad *_tustin(Biquad *analog, float K);

#ifdef __cplusplus
}
#endif
