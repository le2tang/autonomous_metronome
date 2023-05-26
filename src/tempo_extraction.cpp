#include "tempo_extraction.h"

TempoExtraction::~TempoExtraction() {
    if (filterbank_) {
        for (int idx = 0; idx < num_filters_; ++idx) {
            for (int stage = 0; stage < filterbank_.num_stages(); ++idx) {
                delete filterbank_[idx].get_stage(idx);
            }
            delete filterbank_[idx];
        }
        delete[] filterbank_;
    }
    if (pwr_spectrum_) {
        delete[] pwr_spectrum_;
    }
}

void TempoExtraction::init(const TempoExtractionParams &params) {
    num_filters_ = params.num_filters;
    filterbank_ = new IIRFilter[params.num_filters];
    for (int idx = 0; idx < params.num_filters; ++idx) {
        float peak_freq = (params.start_bpm + params.step_bpm * idx) / 60;

        filterbank_[idx] = new IIRFilter(3);
        filterbank_[idx].set_stage(0, iirpeak(0.5 * peak_freq, params.q_factor,  params.sample_rate));
        filterbank_[idx].set_stage(1, iirpeak(peak_freq, 0.25 * params.q_factor,  params.sample_rate));
        filterbank_[idx].set_stage(2, iirpeak(1.5 * peak_freq, 0.5 * params.q_factor,  params.sample_rate));
    }

    pwr_spectrum_ = new float[params.num_filters];
    pwr_decay_ = params.pwr_decay;

    softmax_gain_ = params.softmax_gain;
    softmax_thresh_ = params.softmax_thresh;
}

float TempoExtraction::update(float sample) {
    float prob_sum = 0;
    float softmax[num_filters_];
    for (int idx = 0; idx < num_filters_; ++idx) {
        float y = filterbank_[idx].filter(sample);
        pwr_spectrum_[idx] = (1 - pwr_decay_) * y + pwr_decay_ * pwr_spectrum_[idx];
    
        softmax[idx] = exp(softmax_gain_ * pwr_spectrum_[idx]);
        sum += softmax[idx];
    }
    
    float max_prob = 0;
    int max_idx = 0;
    for (int idx = 0; idx < num_filters_; ++idx) {
        softmax[idx] /= sum;
    
        if (softmax[idx] > softmax_thresh_ && softmax[idx] > max_prob) {
            max_prob = softmax[idx];
            max_idx = idx;
        }
    }

    float freq_est;
    if (max_idx == 0 || max_idx == num_filters_ - 1) {
        freq_est = (start_bpm + step_bpm * max_idx) / 60; 
    }
    else {            
        float f0 = (start_bpm + step_bpm * (max_idx - 1)) / 60;
        float f1 = (start_bpm + step_bpm * max_idx) / 60;
        float f2 = (start_bpm + step_bpm * (max_idx + 1)) / 60;
    
        float f0_sq = f0 * f0;
        float f1_sq = f1 * f1;
        float f2_sq = f2 * f2;

        float lnp0 = log(pwr_spectrum_[max_idx - 1]);
        float lnp1 = log(pwr_spectrum_[max_idx]);
        float lnp2 = log(pwr_spectrum_[max_idx + 1]);

        // float det = f0_sq * (f1 - f2) + f1_sq * (f2 -f0) + f2_sq * (f0 - f1);
        float coef_a = lnp0 * (f1 - f2) + lnp1 * (f2 - f0) + lnp2 * (f0 - f1);
        float coef_b = lnp0 * (f2_sq - f1_sq) + lnp1 * (f0_sq - f2_sq) + lnp2 * (f1_sq - f0_sq);

        freq_est = -0.5 * coef_b / coef_a;
    }
    return freq_est;
}