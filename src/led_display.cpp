#include "led_display.h"

#include "daisy.h"
#include "util.h"

void LedDisplay::init (const LedDisplayParams &params) {
    last_transition_time_ = 0;
    transition_duration_ = params.transition_duration;
    
    last_tempo_ = TempoEstimate(2, 0);
    tempo_ = TempoEstimate(2, 0);

    led_decay_(params.led_decay);
    led_.Init(params.led_pin, false);
}

void LedDisplay::reset_start_millis() {
    start_millis_ = System::GetNow();
}

void LedDisplay::set_tempo(const TempoEstimate &new_tempo) {
    last_tempo_ = tempo_;
    tempo_ = new_tempo;

    last_transition_time_ = 0.001 * System::GetNow();
}

void LedDisplay::update() {
    unsigned long curr_millis = System::GetNow();
    unsigned long elapsed_millis = curr_millis - start_millis_;
    float elapsed_time = 0.001 * elapsed_millis;

    if (0.001 * curr_millis - last_transition_time_ > transition_duration_) {
        float phase = fmod(tempo_.rate * elapsed_time - tempo_.phase, 1);
        unsigned int led_pwr = 256 * exp(-led_decay_ * phase);

        led_.Set(led_pwr);
    }
    else {
        float last_phase_unwrapped = last_tempo_.rate * elapsed_time - last_tempo_.phase;
        float curr_phase_unwrapped = tempo_.rate * elapsed_time - tempo_.phase;
        float wgt = 1 / 1 + exp(-blend_speed_ * (elapsed_time - last_transition_time_ - transition_duration_ / 2));

        float phase = fmod(wgt * last_phase_unwrapped + (1 - wgt) * curr_phase_unwrapped, 1);
        unsigned int led_pwr = 256 * exp(-led_decay_ * phase);
        
        led_.Set(led_pwr);
    }
}
