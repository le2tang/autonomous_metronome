#pragma once

#include "iir_filter.h"

struct DecimatorParams {
    float sample_rate;
    int decimation_factor;
};

#ifdef __cplusplus
extern "C" {
#endif

class Decimator {
  public:
    Decimator() {}
    ~Decimator();

    void init(const DecimatorParams &params);

    float update(float sample, bool &ready);

  private:
    IIRFilter *antialias_filter_;

    int idx_;
    int factor_;
};

#ifdef __cplusplus
}
#endif
