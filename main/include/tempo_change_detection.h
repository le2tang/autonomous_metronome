#pragma once

#include "util.h"

struct TempoChangeDetectionParams {
    float confidence_threshold;
    float gain_threshold;
};

#ifdef __cplusplus
extern "C" {
#endif

class TempoChangeDetection {
  public:
    TempoChangeDetection() {}
    ~TempoChangeDetection() {}

    void init(const TempoChangeDetectionParams &params) {
        confidence_threshold_ = params.confidence_threshold;
        gain_threshold_ = params.gain_threshold;
    }

    bool get_changed_detected(float tempo_confidence,
                              const Buffer &onset_buffer) const {
        float onset_pwr = 0;
        for (int idx = 0; idx < onset_buffer.size(); ++idx) {
            onset_pwr += onset_buffer[idx] * onset_buffer[idx];
        }
        return (tempo_confidence > confidence_threshold_) &&
               (onset_pwr > gain_threshold_);
    }

  private:
    float confidence_threshold_;
    float gain_threshold_;
};

#ifdef __cplusplus
}
#endif
