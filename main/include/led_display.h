#pragma once

#include "led.h"
#include "util.h"

struct LedDisplayParams {
    float transition_duration;

    float led_decay;
    int led_pin;
};

#ifdef __cplusplus
extern "C" {
#endif

class LedDisplay {
  public:
    LedDisplay() {}
    ~LedDisplay() {}

    void init(const LedDisplayParams &params);

    void reset_start_time();

    void set_tempo(const TempoEstimate &new_tempo);
    TempoEstimate &get_tempo();
    const TempoEstimate &get_tempo() const;

    void update();

  private:
    float start_time_;

    float last_transition_time_;
    float transition_duration_;
    float blend_speed_;

    TempoEstimate last_tempo_;
    TempoEstimate tempo_;

    float led_decay_;
    PWMLed led_;
};

#ifdef __cplusplus
}
#endif
