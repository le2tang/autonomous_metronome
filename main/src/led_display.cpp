#include "led_display.h"

#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "util.h"

void LedDisplay::init(const LedDisplayParams &params) {
    last_transition_time_ = 0;
    transition_duration_ = params.transition_duration;

    last_tempo_ = TempoEstimate(2, 0);
    tempo_ = TempoEstimate(2, 0);

    led_decay_(params.led_decay);
}

void LedDisplay::reset_start_millis() {
    start_millis_ = xTaskGetTickCount() / configTICK_RATE_HZ;
}

void LedDisplay::set_tempo(const TempoEstimate &new_tempo) {
    last_tempo_ = tempo_;
    tempo_ = new_tempo;

    last_transition_time_ = xTaskGetTickCount() / configTICK_RATE_HZ;
}

void LedDisplay::update() {
    float curr_time = xTaskGetTickCount() / configTICK_RATE_HZ;
    float elapsed_time = curr_time - start_time_;

    if (curr_time - last_transition_time_ > transition_duration_) {
        float phase = fmod(tempo_.rate * elapsed_time - tempo_.phase, 1);
        float led_pwr = exp(-led_decay_ * phase);

        led_.set(led_pwr);
    } else {
        float last_phase_unwrapped =
            last_tempo_.rate * elapsed_time - last_tempo_.phase;
        float curr_phase_unwrapped = tempo_.rate * elapsed_time - tempo_.phase;
        float wgt =
            1 / 1 + exp(-blend_speed_ * (elapsed_time - last_transition_time_ -
                                         transition_duration_ / 2));

        float phase = fmod(
            wgt * last_phase_unwrapped + (1 - wgt) * curr_phase_unwrapped, 1);
        float led_pwr = exp(-led_decay_ * phase);

        led_.set(led_pwr);
    }
}
