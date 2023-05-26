#include "phase_extraction.h"

#include <math.h>

void PhaseExtraction::init(const PhaseExtractionParams &params,
                           const Buffer &in) {
    num_samples_ = params.num_samples;
    sample_rate_ = params.sample_rate;

    in_buf_ = &in;

    start_time_sample_ = 0;
}

float PhaseExtraction::update(float tempo_freq) {
    float sample_tempo_period = sample_rate_ / tempo_freq;

    float max_mean_pulse = 0;
    float max_pulse_idx = 0;
    for (int idx = 0; idx < sample_tempo_period; ++idx) {
        float pulse_sum = 0;
        int pulse_count = 0;

        float pulse_idx = num_samples_ - 1 - idx;
        while (pulse_idx > 0) {
            pulse_sum += (*in_buf_)[int(pulse_idx)];
            ++pulse_count;
            pulse_idx -= sample_tempo_period;
        }

        float mean_pulse = pulse_sum / pulse_count;
        if (mean_pulse > max_mean_pulse) {
            max_mean_pulse = mean_pulse;
            max_pulse_idx = idx;
        }
    }

    float phase_est =
        fmod((start_time_sample_ + max_pulse_idx) / sample_tempo_period, 1);

    start_time_sample_ += num_samples_;

    return phase_est;
}
