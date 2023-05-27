#include "iir_peak.h"

#include <math.h>

#include "biquad.h"

Biquad *design_iirpeak(float peak_freq, float q_factor, float sample_rate) {
    double norm_peak_freq = 2 * M_PI * peak_freq / sample_rate;
    double bandwidth = norm_peak_freq / q_factor;

    double gain = 1 / (1 + tan(bandwidth / 2));

    double b0 = 1 - gain;
    double b1 = 0;
    double b2 = gain - 1;
    double a1 = -2 * gain * cos(norm_peak_freq);
    double a2 = 2 * gain - 1;

    return new Biquad(b0, b1, b2, a1, a2);
}
