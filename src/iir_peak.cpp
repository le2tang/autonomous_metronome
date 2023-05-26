#include "iir_peak.h"

#include "biquad.h"

Biquad *iirpeak(float peak_freq, float q_factor, float sample_rate) {
    double peak_freq = 2 * M_PI * peak_freq / sample_rate;
    double bandwidth = peak_freq / q_factor;

    double gain = 1 / (1 + tan(bandwidth / 2));

    double b0 = 1 - gain;
    double b1 = 0;
    double b2 = gain - 1;
    double a1 = -2 * gain * cos(peak_freq);
    double a2 = 2 * gain - 1;

    return new Biquad(b0, b1, b2, a1, a2);
}
