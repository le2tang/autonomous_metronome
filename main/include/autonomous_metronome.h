#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "decimator.h"
#include "led_display.h"
#include "onset_detection.h"
#include "phase_extraction.h"
#include "sampler.h"
#include "tempo_extraction.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

class AutonomousMetronome {
  public:
    AutonomousMetronome();
    ~AutonomousMetronome() {}

    void led_display_task();
    void tempo_extraction_task();

    static void led_display_task_impl(void *arg);
    static void tempo_extraction_task_impl(void *arg);

  private:
    TaskHandle_t led_display_task_handle_;
    TaskHandle_t tempo_extraction_task_handle_;

    QueueHandle_t tempo_queue_handle_;

    LedDisplay led_display_;
    OnsetDetection onset_detection_;
    Decimator decimator_;
    TempoExtraction tempo_extraction_;
    PhaseExtraction phase_extraction_;

    Sampler sampler_;
};

#ifdef __cplusplus
}
#endif
