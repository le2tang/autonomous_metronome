#pragma once

#include "daisy.h"
#include "util.h"

struct LedDisplayParams {
    float transition_duration;
    
    float led_decay;
    unsigned int led_pin;
};

class LedDisplay {
public:
    void init (const LedDisplayParams &params);

    void reset_start_millis();

    void set_tempo(const TempoEstimate &new_tempo);
    TempoEstimate &get_tempo();
    const TempoEstimate &get_tempo() const;

    void update();

private:
    unsigned long start_millis_;
    
    float last_transition_time_;
    float transition_duration_;

    TempoEstimate last_tempo_;
    TempoEstimate tempo_;

    float led_decay_;
    daisy::Led led_;
};
