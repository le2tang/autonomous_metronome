#pragma once

#include <math.h>

#include "esp_log.h"

#include "design_iir.h"
#include "iir_filter.h"
#include "util.h"

struct SilenceDetectorParams {
    float filter_freq;
    float sample_rate;

    float threshold;
};

class SilenceDetector {
  public:
    SilenceDetector() {}
    ~SilenceDetector() {
        if (gain_filter_) {
            delete gain_filter_->get_stage(0);
            delete gain_filter_;
        }
    }

    void init(const SilenceDetectorParams &params) {
        gain_filter_ = new IIRFilter(1);
        gain_filter_->set_stage(
            0, design_butterlp(params.filter_freq, params.sample_rate));

        threshold_ = params.threshold;
    }

    void update(float sample) {
        float log_gain = (sample > 1E-9) ? log(sample) : log(1E-9);
        gain_ = gain_filter_->filter(log_gain);
    }

    inline bool is_silent() const { return gain_ < threshold_; }

    inline float get_norm_gain() const { return exp(-gain_ + 1); }

  private:
    float gain_;
    IIRFilter *gain_filter_;

    float threshold_;
};
