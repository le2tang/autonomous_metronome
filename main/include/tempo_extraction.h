#pragma once

#include "iir_filter.h"

struct TempoExtractionParams {
    float start_bpm;
    float step_bpm;

    float filter_res_factor;
    float sample_rate;

    int num_filters;
    float pwr_decay;

    float lowpass_freq;

    float softmax_gain;
    float softmax_thresh;
};

#ifdef __cplusplus
extern "C" {
#endif

class TempoExtraction {
  public:
    TempoExtraction() {}
    ~TempoExtraction();

    void init(const TempoExtractionParams &params);

    float update(float sample);

  private:
    int num_filters_;
    IIRFilter **filterbank_;

    float smooth_coef_[2];
    float *smooth_delay_;
    float *pwr_spectrum_;
    float pwr_decay_;

    float softmax_gain_;
    float softmax_ovflo_;
    float softmax_thresh_;

    float start_bpm_;
    float step_bpm_;

    float freq_est_;
};

#ifdef __cplusplus
}
#endif
