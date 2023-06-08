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
        filterbank_[idx]->set_stage(2, design_reslp(0.5 * peak_freq,
                                                    params.filter_res_factor,
                                                    params.sample_rate));
        filterbank_[idx]->set_stage(
            1, design_reslp(peak_freq, pow(params.filter_res_factor, 2),
                            params.sample_rate));
        filterbank_[idx]->set_stage(
            0, design_resbp(1.5 * peak_freq, pow(params.filter_res_factor, 3),
                            params.sample_rate));

        float unit_gain_freq = 2 * M_PI * peak_freq / params.sample_rate;
        float gain =
            filterbank_[idx]->get_stage(0)->eval_freq_gain(unit_gain_freq) *
            filterbank_[idx]->get_stage(1)->eval_freq_gain(unit_gain_freq) *
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

    start_bpm_ = params.start_bpm;
    step_bpm_ = params.step_bpm;
}

TempoExtractionResult TempoExtraction::update(float sample) {
    float raw[num_filters_];
    float smooth[num_filters_];
    for (int idx = 0; idx < num_filters_; ++idx) {
        float y = filterbank_[idx]->filter(sample);

        float smooth_y_temp = fabs(y) - smooth_coef_[1] * smooth_delay_[idx];
        float smooth_y = smooth_coef_[0] * (smooth_y_temp + smooth_delay_[idx]);
        smooth_delay_[idx] = smooth_y_temp;

        pwr_spectrum_[idx] = pwr_decay_ * smooth_y * smooth_y +
                             (1 - pwr_decay_) * pwr_spectrum_[idx];

        raw[idx] = sample;
        smooth[idx] = smooth_y;
    }

    float max_pwr = 0;
    int max_idx = 0;
    for (int idx = 0; idx < num_filters_; ++idx) {
        if (pwr_spectrum_[idx] > max_pwr) {
            max_pwr = pwr_spectrum_[idx];
            max_idx = idx;
        }
    }

    float exp_sum_norm = 0;
    for (int idx = 0; idx < num_filters_; ++idx) {
        exp_sum_norm += exp(pwr_spectrum_[idx] - max_pwr);
    }

    float softmax[num_filters_];
    for (int idx = 0; idx < num_filters_; ++idx) {
        float log_sum_exp = max_pwr + log(exp_sum_norm);
        softmax[idx] = exp(pwr_spectrum_[idx] - log_sum_exp);
    }

    TempoExtractionResult result;
    if (max_idx == 0 || max_idx == num_filters_ - 1) {
        result.freq = (start_bpm_ + step_bpm_ * max_idx) / 60;
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
        float coef_a = lnp0 * (f1 - f2) + lnp1 * (f2 - f0) + lnp2 * (f0 - f1);
        float coef_b = lnp0 * (f2_sq - f1_sq) + lnp1 * (f0_sq - f2_sq) +
                       lnp2 * (f1_sq - f0_sq);

        result.freq = -0.5 * coef_b / coef_a;
    }
    // ESP_LOGI("TE", "%07d %07d %07d %03d %d", (int)(1E0 * raw[max_idx]),
    //          (int)(1E0 * smooth[max_idx]), (int)(1E0 *
    //          pwr_spectrum_[max_idx]), (int)(100 * softmax[max_idx]),
    //          (int)(result.freq * 60));

    result.confidence = softmax[max_idx];
    return result;
}
