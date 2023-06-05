#include "autonomous_metronome.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/stream_buffer.h"
#include "freertos/task.h"

#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

AutonomousMetronome::AutonomousMetronome() {
    tempo_queue_handle_ = xQueueCreate(2, sizeof(TempoEstimate));

    xTaskCreate(led_display_task_impl, "led_display", 1024, this, 1,
                &led_display_task_handle_);
    xTaskCreate(tempo_extraction_task_impl, "tempo_extraction", 4096, this, 0,
                &tempo_extraction_task_handle_);
}

void AutonomousMetronome::led_display_task_impl(void *arg) {
    static_cast<AutonomousMetronome *>(arg)->led_display_task();
}

void AutonomousMetronome::tempo_extraction_task_impl(void *arg) {
    static_cast<AutonomousMetronome *>(arg)->tempo_extraction_task();
}

void AutonomousMetronome::led_display_task() {
    led_display_.init(
        LedDisplayParams{.transition_duration = 4, .led_decay = 10.0});

    // Turnstyle
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xTaskNotifyGive(tempo_extraction_task_handle_);
    ESP_LOGI("AM", "LED init");

    led_display_.reset_start_time();

    TempoEstimate new_tempo;
    while (1) {
        if (xQueueReceive(tempo_queue_handle_, &new_tempo, 0)) {
            ESP_LOGI("LED", "New tempo: %dBPM", (int)(new_tempo.rate * 60));
            led_display_.set_tempo(new_tempo);
        }

        led_display_.update();

        // ESP_LOGI("LD", "%d", xTaskGetTickCount() * portTICK_RATE_MS);

        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

void AutonomousMetronome::tempo_extraction_task() {
    const float sample_rate = 10000;
    const int sample_period_us = 1E6 / sample_rate;
    const int hop_size = 128;
    const float hop_rate = sample_rate / hop_size;
    const int sample_byte_size = sizeof(uint16_t);
    const int buffer_byte_size = hop_size * sample_byte_size;
    const int downsample_factor = 8;
    const float downsample_rate = hop_rate / downsample_factor;
    const int onset_history_size = 128;
    const float onset_gain = 100;

    Buffer onset_buffer(onset_history_size);

    onset_detection_.init(OnsetDetectionParams{.num_samples = hop_size,
                                               .num_bands = hop_size / 2});
    decimator_.init(DecimatorParams{.sample_rate = hop_rate,
                                    .decimation_factor = downsample_factor});
    median_filter_.init(8);
    tempo_extraction_.init(TempoExtractionParams{.start_bpm = 60,
                                                 .step_bpm = 10,
                                                 .num_filters = 9,
                                                 .filter_res_factor = 0.99,
                                                 .sample_rate = downsample_rate,
                                                 .pwr_decay = 0.1,
                                                 .lowpass_freq = 0.35,
                                                 .softmax_gain = 100,
                                                 .softmax_thresh = 0.5});
    silence_detector_.init(SilenceDetectorParams{
        .filter_freq = 0.05, .sample_rate = downsample_rate, .threshold = -2});
    phase_extraction_.init(
        PhaseExtractionParams{.num_samples = onset_history_size,
                              .sample_rate = downsample_rate},
        onset_buffer);
    sampler_.init(SamplerParams{.buffer_size = 4 * hop_size,
                                .buffer_trigger = hop_size,
                                .sample_byte_size = sample_byte_size,
                                .sample_period_us = sample_period_us});

    // Turnstyle
    xTaskNotifyGive(led_display_task_handle_);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    ESP_LOGI("AM", "TE init");

    // Start sampler
    sampler_.start_timer();

    uint16_t buf[hop_size];
    while (1) {
        xStreamBufferReceive(sampler_.get_stream_buffer(), buf,
                             buffer_byte_size, portMAX_DELAY);

        onset_detection_.load_input(buf);
        float onset = onset_detection_.update() - median_filter_.median();
        if (onset < 0) {
            onset = 0;
        }
        // ESP_LOGI("AM", "%d", (int)(1E6 * onset));

        bool decimate_sample_ready = false;
        onset = decimator_.update(onset, decimate_sample_ready);

        if (decimate_sample_ready) {
            median_filter_.update(onset);
            silence_detector_.update(onset);

            onset *= onset_gain * silence_detector_.get_norm_gain();

            float tempo_freq = tempo_extraction_.update(onset);
            onset_buffer.push(onset);

            if (onset_buffer.full()) {
                if (!silence_detector_.is_silent()) {
                    // float phase = phase_extraction_.update(tempo_freq);

                    // TempoEstimate new_tempo{.rate = tempo_freq, .phase =
                    // phase}; xQueueSend(tempo_queue_handle_, &new_tempo,
                    //            10 / portTICK_RATE_MS);
                }

                onset_buffer.reset();
            }
        }
    }
}

#ifdef __cplusplus
}
#endif
