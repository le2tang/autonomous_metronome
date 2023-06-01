#pragma once

#include "biquad.h"

#ifdef __cplusplus
extern "C" {
#endif

Biquad *design_iirpeak(float peak_freq, float q_factor, float sample_rate);
Biquad *design_reslp(float peak_freq, float resonant_factor, float sample_rate);
Biquad *design_resbp(float peak_freq, float resonant_factor, float sample_rate);
Biquad *design_butterlp(float cutoff_freq, float sample_rate);

#ifdef __cplusplus
}
#endif
