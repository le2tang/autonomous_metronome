#include "tempo_extraction.h"

#include <cstring>
#include <math.h>

#include "esp_log.h"

#include "design_iir.h"
#include "iir_filter.h"
#include "memory.h"

TempoExtraction::~TempoExtraction() {
    if (smooth_delay_) {
        delete[] smooth_delay_;
    }
    if (filterbank_) {
        for (int idx = 0; idx < num_filters_; ++idx) {
            for (int stage = 0; stage < filterbank_[idx]->num_stages(); ++idx) {
                delete filterbank_[idx]->get_stage(idx);
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
    filterbank_ = new IIRFilter *[params.num_filters];
    for (int idx = 0; idx < params.num_filters; ++idx) {
        float peak_freq = (params.start_bpm + params.step_bpm * idx) / 60;

        filterbank_[idx] = new IIRFilter(3);
        filterbank_[idx]->set_stage(0, design_reslp(0.5 * peak_freq,
                                                    params.filter_res_factor,
                                                    params.sample_rate));
        filterbank_[idx]->set_stage(1, design_reslp(peak_freq,
                                                    params.filter_res_factor,
                                                    params.sample_rate));
        filterbank_[idx]->set_stage(2, design_resbp(1.5 * peak_freq,
                                                    params.filter_res_factor,
                                                    params.sample_rate));

        float unit_gain_freq = 2 * M_PI * peak_freq / params.sample_rate;
        float gain =
            filterbank_[idx]->get_stage(0)->eval_freq_gain(unit_gain_freq) *
            filterbank_[idx]->get_stage(2)->eval_freq_gain(unit_gain_freq);
        filterbank_[idx]->set_gain(1 / gain);
    }

    double norm_freq = M_PI * params.lowpass_freq / params.sample_rate;
    smooth_coef_[0] = norm_freq / (norm_freq + 1);
    smooth_coef_[1] = (norm_freq - 1) / (norm_freq + 1);
    smooth_delay_ = new float[params.num_filters];
    memset(smooth_delay_, 0, params.num_filters * sizeof(float));

    pwr_spectrum_ = new float[params.num_filters];
    memset(pwr_spectrum_, 0, params.num_filters * sizeof(float));
    pwr_decay_ = params.pwr_decay;

    softmax_gain_ = params.softmax_gain;
    softmax_ovflo_ = log((float)(1ULL << 31)) / params.softmax_gain;
    softmax_thresh_ = params.softmax_thresh;

    start_bpm_ = params.start_bpm;
    step_bpm_ = params.step_bpm;

    freq_est_ = 1.0;
}

float TempoExtraction::update(float sample) {
    float prob_sum = 0;
    float softmax[num_filters_];
    for (int idx = 0; idx < num_filters_; ++idx) {
        float y = filterbank_[idx]->filter(sample);

        float smooth_y_temp = fabs(y) - smooth_coef_[1] * smooth_delay_[idx];
        float smooth_y = smooth_coef_[0] * (smooth_y_temp + smooth_delay_[idx]);
        smooth_delay_[idx] = smooth_y_temp;

        pwr_spectrum_[idx] = (1 - pwr_decay_) * smooth_y * smooth_y +
                             pwr_decay_ * pwr_spectrum_[idx];

        float softmax_arg = ((pwr_spectrum_[idx]) > softmax_ovflo_)
                                ? softmax_ovflo_
                                : softmax_gain_ * pwr_spectrum_[idx];
        softmax[idx] = exp(softmax_arg);
        prob_sum += softmax[idx];
    }

    float max_prob = 0;
    int max_idx = 0;
    for (int idx = 0; idx < num_filters_; ++idx) {
        softmax[idx] /= prob_sum;

        if (pwr_spectrum_[idx] > max_prob) {
            max_prob = pwr_spectrum_[idx];
            max_idx = idx;
        }
    }

    ESP_LOGI("TE", "%07d %07d %07d %07d %07d %07d %07d %07d %d %d",
             (int)(1E6 * pwr_spectrum_[0]), (int)(1E6 * pwr_spectrum_[1]),
             (int)(1E6 * pwr_spectrum_[2]), (int)(1E6 * pwr_spectrum_[3]),
             (int)(1E6 * pwr_spectrum_[4]), (int)(1E6 * pwr_spectrum_[5]),
             (int)(1E6 * pwr_spectrum_[6]), (int)(1E6 * pwr_spectrum_[7]),
             (int)(1000 * log10(pwr_spectrum_[1])), max_idx);
    // ESP_LOGI("TE", "%03d %03d %03d %03d %03d %03d %03d %03d %d",
    //          (int)(100 * softmax[0]), (int)(100 * softmax[1]),
    //          (int)(100 * softmax[2]), (int)(100 * softmax[3]),
    //          (int)(100 * softmax[4]), (int)(100 * softmax[5]),
    //          (int)(100 * softmax[6]), (int)(100 * softmax[7]), max_idx);

    if (softmax[max_idx] > softmax_thresh_) {
        if (max_idx == 0 || max_idx == num_filters_ - 1) {
            freq_est_ = (start_bpm_ + step_bpm_ * max_idx) / 60;
        } else {
            float f0 = (start_bpm_ + step_bpm_ * (max_idx - 1)) / 60;
            float f1 = (start_bpm_ + step_bpm_ * max_idx) / 60;
            float f2 = (start_bpm_ + step_bpm_ * (max_idx + 1)) / 60;

            float f0_sq = f0 * f0;
            float f1_sq = f1 * f1;
            float f2_sq = f2 * f2;

            float lnp0 = log(pwr_spectrum_[max_idx - 1]);
            float lnp1 = log(pwr_spectrum_[max_idx]);
            float lnp2 = log(pwr_spectrum_[max_idx + 1]);

            // float det = f0_sq * (f1 - f2) + f1_sq * (f2 -f0) + f2_sq *
            // (f0 - f1);
            float coef_a =
                lnp0 * (f1 - f2) + lnp1 * (f2 - f0) + lnp2 * (f0 - f1);
            float coef_b = lnp0 * (f2_sq - f1_sq) + lnp1 * (f0_sq - f2_sq) +
                           lnp2 * (f1_sq - f0_sq);

            freq_est_ = -0.5 * coef_b / coef_a;
        }
        // ESP_LOGI("TE", "%dBPM", (int)(60 * freq_est_));
    }
    return freq_est_;
}