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
    sync_queue_handle_ = xQueueCreate(2, sizeof(unsigned char));
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
    led_display_.init(LedDisplayParams{
        .transition_duration = 8, .led_decay = 10, .led_pin = 5});

    // Turnstyle
    xTaskNotifyGive(tempo_extraction_task_handle_);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    ESP_LOGI("AM", "LED init");

    led_display_.reset_start_time();

    unsigned char sync;
    TempoEstimate new_tempo;
    while (1) {
        if (xQueueReceive(sync_queue_handle_, &sync, 0)) {
            xTaskNotifyGive(tempo_extraction_task_handle_);
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            led_display_.reset_start_time();

            ESP_LOGI("AM", "Reset LED time");
        }

        if (xQueueReceive(tempo_queue_handle_, &new_tempo, 0)) {
            ESP_LOGI("LED", "New tempo: %dBPM", (int)(new_tempo.rate * 60));
            led_display_.set_tempo(new_tempo);
        }

        led_display_.update();

        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

void AutonomousMetronome::tempo_extraction_task() {
    const float sample_rate = 10000;
    const unsigned int sample_period_us = 1E6 / sample_rate;
    const int hop_size = 128;
    const float hop_rate = sample_rate / hop_size;
    const int sample_byte_size = sizeof(uint16_t);
    const int buffer_byte_size = hop_size * sample_byte_size;
    const int downsample_factor = 8;
    const float downsample_rate = hop_rate / downsample_factor;
    const int onset_history_size = 128;

    uint16_t sample_buffer[hop_size];
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
                                                 .softmax_gain = 0.001});
    auto_gain_detection_.init(
        AutoGainDetectionParams{.filter_freq = 0.1, .sample_rate = hop_rate});
    tempo_change_detection_.init(TempoChangeDetectionParams{
        .confidence_threshold = 0.5, .gain_threshold = 3});
    phase_extraction_.init(
        PhaseExtractionParams{.num_samples = onset_history_size,
                              .sample_rate = downsample_rate},
        onset_buffer);
    sampler_.init(SamplerParams{.buffer_size = 2 * hop_size * sample_byte_size,
                                .buffer_trigger = hop_size,
                                .sample_byte_size = sample_byte_size,
                                .sample_period_us = sample_period_us});
    button_.init(4);
    Led led;
    led.init(0);

    // Turnstyle
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xTaskNotifyGive(led_display_task_handle_);

    while (1) {
        if (button_.get_status()) {
            led.set(true);
            float start_time = xTaskGetTickCount() * portTICK_RATE_MS;

            xStreamBufferReset(sampler_.get_stream_buffer());
            onset_buffer.reset();
            phase_extraction_.reset_start_time();

            unsigned char sync = 1;
            xQueueSend(sync_queue_handle_, &sync, 10 / portTICK_RATE_MS);

            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            xTaskNotifyGive(led_display_task_handle_);

            sampler_.start_timer();

            TempoExtractionResult tempo;
            while (!onset_buffer.full()) {
                if (xStreamBufferBytesAvailable(sampler_.get_stream_buffer()) <
                    sampler_.get_trigger_byte_size()) {
                    vTaskDelay(1);
                    continue;
                }
                xStreamBufferReceive(sampler_.get_stream_buffer(),
                                     sample_buffer, buffer_byte_size,
                                     portMAX_DELAY);
                onset_detection_.load_input(sample_buffer);

                float onset = onset_detection_.update();
                auto_gain_detection_.update(onset);

                onset -= median_filter_.get_median();
                if (onset < 0) {
                    onset = 0;
                }

                bool decimate_sample_ready = false;
                onset = decimator_.update(onset, decimate_sample_ready);

                if (decimate_sample_ready) {
                    median_filter_.update(onset);

                    onset *= auto_gain_detection_.get_norm_gain();
                    onset_buffer.push(onset);

                    tempo = tempo_extraction_.update(onset);
                }
            }

            if (tempo_change_detection_.get_changed_detected(tempo.confidence,
                                                             onset_buffer)) {
                float phase = phase_extraction_.update(tempo.freq);

                TempoEstimate new_tempo{.rate = tempo.freq, .phase = phase};
                xQueueSend(tempo_queue_handle_, &new_tempo,
                           10 / portTICK_RATE_MS);
            }

            sampler_.stop_timer();

            float end_time = xTaskGetTickCount() * portTICK_RATE_MS;
            led.set(false);
        }

        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

#ifdef __cplusplus
}
#endif
