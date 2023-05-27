#pragma once

#include "biquad.h"

#ifdef __cplusplus
extern "C" {
#endif

Biquad *design_iirpeak(float peak_freq, float q_factor, float sample_rate);

#ifdef __cplusplus
}
#endif
