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

    xTaskCreate(led_display_task_impl, "led_display", 1024, this, 2,
                &led_display_task_handle_);
    xTaskCreate(tempo_extraction_task_impl, "tempo_extraction", 4096, this, 3,
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
        .transition_duration = 1.5, .blend_speed = 0.5, .led_decay = 10.0});

    // Turnstyle
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xTaskNotifyGive(tempo_extraction_task_handle_);

    led_display_.reset_start_time();

    TempoEstimate new_tempo;
    while (1) {
        // if (xQueueReceive(tempo_queue_handle_, &new_tempo, 0)) {
        //     led_display_.set_tempo(new_tempo);
        // }

        // ESP_LOGI("LED", "%d", xTaskGetTickCount());
        led_display_.update();

        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

void AutonomousMetronome::tempo_extraction_task() {
    const float sample_rate = 10000;
    const int decimation_factor = 64;
    const float decimation_rate = sample_rate / decimation_factor;
    const int sample_byte_size = sizeof(uint16_t);
    const int buffer_byte_size = decimation_factor * sample_byte_size;
    const int onset_history = 128;

    Buffer onset_buffer(onset_history);

    onset_detection_.init(OnsetDetectionParams{
        .num_samples = decimation_factor, .num_bands = decimation_factor / 2});
    tempo_extraction_.init(TempoExtractionParams{.start_bpm = 50,
                                                 .step_bpm = 10,
                                                 .filter_q_factor = 30,
                                                 .sample_rate = decimation_rate,
                                                 .num_filters = 10,
                                                 .pwr_decay = 0.01,
                                                 .softmax_gain = 100,
                                                 .softmax_thresh = 0.5});
    phase_extraction_.init(
        PhaseExtractionParams{.num_samples = onset_history,
                              .sample_rate = decimation_rate},
        onset_buffer);
    sampler_.init(SamplerParams{.buffer_size = 4 * decimation_factor,
                                .buffer_trigger = decimation_factor,
                                .sample_byte_size = sample_byte_size,
                                .sample_period_us = 100000});

    // Turnstyle
    xTaskNotifyGive(led_display_task_handle_);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Start sampler
    // sampler_.start_timer();

    uint16_t buf[decimation_factor];
    while (1) {
        // xStreamBufferReceive(sampler_.get_stream_buffer(), buf,
        //                      buffer_byte_size, portMAX_DELAY);

        // onset_detection_.load_input(buf);

        // float onset = onset_detection_.update();
        // float tempo_freq = tempo_extraction_.update(onset);

        // onset_buffer.push(onset);
        // if (onset_buffer.full()) {
        //     float phase = phase_extraction_.update(tempo_freq);

        //     TempoEstimate new_tempo{.rate = tempo_freq, .phase = phase};
        //     xQueueSend(tempo_queue_handle_, &new_tempo, 10 /
        //     portTICK_RATE_MS);

        //     onset_buffer.reset();
        // }
        // ESP_LOGI("OD", "%d", xTaskGetTickCount());

        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

#ifdef __cplusplus
}
#endif
