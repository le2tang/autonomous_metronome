#include "design_iir.h"

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

Biquad *design_reslp(float peak_freq, float resonant_factor,
                     float sample_rate) {
    double norm_peak_freq = 2 * M_PI * peak_freq / sample_rate;

    double b0 = 1;
    double b1 = 2;
    double b2 = 1;

    double a1 = -2 * resonant_factor * cos(norm_peak_freq);
    double a2 = resonant_factor * resonant_factor;

    Biquad unnormalized(b0, b1, b2, a1, a2);
    double peak_gain = unnormalized.eval_freq_gain(norm_peak_freq);

    return new Biquad(b0 / peak_gain, b1 / peak_gain, b2 / peak_gain, a1, a2);
}

Biquad *design_resbp(float peak_freq, float resonant_factor,
                     float sample_rate) {
    double norm_peak_freq = 2 * M_PI * peak_freq / sample_rate;

    double b0 = 1;
    double b1 = 0;
    double b2 = -resonant_factor;

    double a1 = -2 * resonant_factor * cos(norm_peak_freq);
    double a2 = resonant_factor * resonant_factor;

    Biquad unnormalized(b0, b1, b2, a1, a2);
    double peak_gain = unnormalized.eval_freq_gain(norm_peak_freq);

    return new Biquad(b0 / peak_gain, b1 / peak_gain, b2 / peak_gain, a1, a2);
}

Biquad *design_butterlp(float cutoff_freq, float sample_rate) {
    double rad_cutoff_freq = 2 * M_PI * cutoff_freq;
    Biquad analog(0, 0, rad_cutoff_freq * rad_cutoff_freq,
                  rad_cutoff_freq * sqrt(2), rad_cutoff_freq * rad_cutoff_freq);
    return warped_tustin(&analog, sample_rate, cutoff_freq);
}
