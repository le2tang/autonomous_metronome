#pragma once

#include <math.h>

#include "esp_log.h"

#include "design_iir.h"
#include "iir_filter.h"
#include "util.h"

struct AutoGainDetectionParams {
    float filter_freq;
    float sample_rate;
};

#ifdef __cplusplus
extern "C" {
#endif

class AutoGainDetection {
  public:
    AutoGainDetection() {}
    ~AutoGainDetection() {
        if (gain_filter_) {
            delete gain_filter_->get_stage(0);
            delete gain_filter_;
        }
    }

    void init(const AutoGainDetectionParams &params) {
        gain_filter_ = new IIRFilter(1);
        gain_filter_->set_stage(
            0, design_butterlp(params.filter_freq, params.sample_rate));
    }

    void update(float sample) {
        float log_gain = (sample > 1E-9) ? log(sample) : log(1E-9);
        gain_ = gain_filter_->filter(log_gain);
        // if (log_gain > gain_) {
        //     float steady_state_delay = 1 + gain_filter_->get_stage(0)->a1() +
        //                                gain_filter_->get_stage(0)->a2();
        //     gain_filter_->get_stage(0)->set_initial_conditions(
        //         1 / steady_state_delay, 1 / steady_state_delay);
        //     gain_ = gain_filter_->filter(log_gain);
        // } else {
        //     gain_ = gain_filter_->filter(log_gain);
        // }
    }

    inline float get_log_gain() const { return gain_; }
    inline float get_norm_gain() const { return exp(-gain_); }

  private:
    float gain_;
    IIRFilter *gain_filter_;
};

#ifdef __cplusplus
}
#endif
