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

    inline void set_gain(float gain) { gain_ = gain; }
    inline float get_gain() const { return gain_; }

    float filter(float x);

  protected:
    const int num_stages_;
    Biquad **stages_;

    float gain_;
};

#ifdef __cplusplus
}
#endif
