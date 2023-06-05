#pragma once

struct TempoChangeDetectionParams {
    float confidence_threshold;
    float gain_threshold;
};

#ifdef __cplusplus
extern "C" {
#endif

class TempoChangeDetection {
  public:
    static const unsigned char TEMPO_CONFIDENT_BIT = 0;
    static const unsigned char ABOVE_GAIN_THRESH_BIT = 1;

    static const unsigned char TEMPO_CONFIDENT_MASK =
        (1U << TEMPO_CONFIDENT_BIT);
    static const unsigned char ABOVE_GAIN_THRESH_MASK =
        (1U << ABOVE_GAIN_THRESH_BIT);

    TempoChangeDetection() : status_(0) {}
    ~TempoChangeDetection() {}

    void init(const TempoChangeDetectionParams &params) {
        confidence_threshold_ = params.confidence_threshold;
        gain_threshold_ = params.gain_threshold;
    }

    void update(float tempo_confidence, float log_gain) {
        set_tempo_confident(tempo_confidence > confidence_threshold_);
        set_above_gain_thresh(log_gain > gain_threshold_);
    }

    void set_tempo_confident(bool is_tempo_confident) {
        status_ = (status_ & ~TEMPO_CONFIDENT_MASK) |
                  (is_tempo_confident << TEMPO_CONFIDENT_BIT);
    }

    void set_above_gain_thresh(bool is_above_gain_thresh) {
        status_ = (status_ & ~ABOVE_GAIN_THRESH_MASK) |
                  (is_above_gain_thresh << ABOVE_GAIN_THRESH_BIT);
    }

    bool get_changed_detected() const {
        return status_ == (TEMPO_CONFIDENT_MASK | ABOVE_GAIN_THRESH_MASK);
    }

  private:
    float confidence_threshold_;
    float gain_threshold_;

    unsigned char status_;
};

#ifdef __cplusplus
}
#endif
