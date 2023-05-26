#include "led_display.h"
#include "onset_detection.h"
#include "phase_extraction.h"
#include "tempo_extraction.h"
#include "util.h"

LedDisplay led_display;
OnsetDetection onset_detection;
TempoExtraction tempo_extraction;
PhaseExtraction phase_extraction;

Buffer onset_buffer;

static void audio_callback(daisysp::AudioHandle::InputBuffer in,
                           daisysp::AudioHandle::OutputBuffer out,
                           size_t size) {
    float onset = onset_detection.update(in);
    float tempo_freq = tempo_extraction.update(onset);

    onset_buffer.push(onset);
    if (onset_buffer.full()) {
        float phase = phase_extraction.update(tempo_freq);

        led_display.set_tempo(TempoEstimate(tempo_freq, phase));

        onset_buffer.reset();
    }
}

int main() {
    float sample_rate = hw.AudioSampleRate();
    float decimation_rate = hw.AudioCallbackRate();

    Buffer onset_buffer(128);

    led_display.init(LedDisplayParams(transition_duration = 1.5,
                                      led_decay = 10.0, led_pin = 9));
    onset_detection.init(
        OnsetDetectionParams(num_bands = hw.AudioBlockSize();));
    tempo_extraction.init(TempoExtractionParams(
        start_bpm = 50, step_bpm = 10, filter_q_factor = 30,
        sample_rate = decimation_rate, num_filters = 10, pwr_decay = 0.01,
        softmax_gain = 100, softmax_thresh = 0.5));
    phase_extraction.init(PhaseExtractionParams(
        num_samples = onset_buffer.num_samples, sample_rate = decimation_rate));

    led_display.reset_start_millis();
    hw.StartAudio(audio_callback);

    while (1) {
        led_display.update();
    }
}
