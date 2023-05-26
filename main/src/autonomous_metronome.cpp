#include "autonomous_metronome.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/stream_buffer.h"
#include "freertos/task.h"

AutonomousMetronome::AutonomousMetronome() {
    tempo_queue_handle_ = xQueueCreate(2, sizeof(TempoEstimate));

    xTaskCreate(led_display_task, "led_display", 1024, NULL, 2,
                led_display_task_handle);
    xTaskCreate(tempo_extraction_task, "tempo_extraction", 2048, NULL, 3,
                tempo_extraction_task_handle_);
}

void AutonomousMetronome::led_display_task(void *arg) {
    led_display_.init(LedDisplayParams(
        transition_duration = 1.5, led_decay = 10.0, led_pin = GPIO_NUM_16));

    // Turnstyle
    xTaskNotifyWait(0, ULONG_MAX, NULL, portMAX_DELAY);
    xTaskNotify(led_display_task_handle_, 0, eNoAction);

    led_display_.reset_start_millis();

    TempoEstimate new_tempo;
    while (1) {
        if (xQueueReceive(tempo_queue_handle_, &new_tempo, 0)) {
            led_display_.set_tempo(new_tempo);
        }

        led_display_.update();

        vTaskDelay(100 / portTICK_RATE_MS)
    }
}

void AutonomousMetronome::tempo_extraction_task(void *arg) {
    onset_detection.init(
        OnsetDetectionParams(num_bands = hw.AudioBlockSize();));
    tempo_extraction.init(TempoExtractionParams(
        start_bpm = 50, step_bpm = 10, filter_q_factor = 30,
        sample_rate = decimation_rate, num_filters = 10, pwr_decay = 0.01,
        softmax_gain = 100, softmax_thresh = 0.5));
    phase_extraction.init(PhaseExtractionParams(
        num_samples = onset_buffer.num_samples, sample_rate = decimation_rate));
    sampler_.init(SamplerParams(buffer_size = 256, buffer_trigger = 64,
                                sizeof(uint16_t)));

    // Turnstyle
    xTaskNotify(led_display_task_handle_, 0, eNoAction);
    xTaskNotifyWait(0, ULONG_MAX, NULL, portMAX_DELAY);

    // Start sampler
    sampler_.start_timer();

    while (1) {
        xStreamBufferReceive(
            sampler_.get_stream_buffer(), onset_detection_.input(),
            onset_detection.num_samples() * sizeof(uint16_t), portMAX_DELAY);

        float onset = onset_detection_.update();
        float tempo_freq = tempo_detection.update();

        onset_buffer.push(onset);
        if (onset_buffer.full()) {
            float phase = phase_extraction.update(tempo_freq);

            TempoEstimate new_tempo{.rate = tempo_freq, .phase = phase};
            xQueueSend(tempo_queue_handle_, &new_tempo, 10 / portTICK_RATE_MS);

            onset_buffer.reset();
        }
    }
}
