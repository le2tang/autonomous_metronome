#pragma once

#include "iir_filter.h"

struct TempoExtractionParams {
    float start_bpm;
    float step_bpm;

    float filter_q_factor;
    float sample_rate;

    int num_filters;
    float pwr_decay;

    float softmax_gain;
    float softmax_thresh;
};

class TempoExtraction {
  public:
    TempoExtraction() {}
    ~TempoExtraction();

    void init(const TempoExtractionParams &params);

    float update(float sample);

  private:
    int num_filters_;
    IIRFilter **filterbank_;

    float *pwr_spectrum_;
    float pwr_decay_;

    float softmax_gain_;
    float softmax_thresh_;

    float start_bpm_;
    float step_bpm_;

    float freq_est_;
};
