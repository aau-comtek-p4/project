#include "esp32/common/context.h"
#include "esp32/common/loggers/esp_logger.h"
#include "esp32/node/context.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "general/awaiters/sleep_for.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include "nvs_flash.h"
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <sys/types.h>

Job do_stuff(int i) {
  while (true) {
    program_ctxt->logger->log_info("TAG", "I am logging, [%i]", i);
    co_await sleep_for(program_ctxt->clock->ms_to_tick(200));
  }
}
Job spawner() {
  int count = 0;
  while (true) {
    auto res = spawn(do_stuff(count));
    count++;
    co_await sleep_for(program_ctxt->clock->ms_to_tick(10000));
  }
}

extern "C" void task1(void *args) {
  ContextConfig<ESPNodeContextSettings> ctx_config;
  ctx_config.clock_type = CtxtClockType::WALL;
  ctx_config.ctx_type = ContextType::NODE;
  ctx_config.io_type = CtxtIOType::DUMMY_IO;
  ctx_config.logger_type = CtxtLoggerType::ESP_LOGGER;
  ctx_config.random_type = CtxtRandomType::NONE;
  ctx_config.deadline_tracker_type = CtxtDeadlineTrackerType::MIN_HEAP;
  ctx_config.random_intervals[RandomType::IO_LATENCY] =
      RandomInterval{.min = 2, .max = 5};
  ctx_config.random_intervals[RandomType::MISSED_TICK] =
      RandomInterval{.min = 1, .max = 3};
  ctx_config.random_intervals[RandomType::MISSED_TICK_CHANCE] =
      RandomInterval{.min = 0, .max = 500};
  ctx_config.random_intervals[RandomType::NETWORK_LATENCY] =
      RandomInterval{.min = 20, .max = 50};
  ctx_config.random_intervals[RandomType::PACKET_DROP] =
      RandomInterval{.min = 0, .max = 200};
  ctx_config.metric_type = CtxtMetricType::STANDARD_METRIC;

  uint8_t *program_buffer =
      (uint8_t *)calloc(ctx_config.settings.max_total_size, sizeof(uint8_t));
  ArenaAllocator program_allocator(program_buffer,
                                   ctx_config.settings.max_total_size);
  ProgramContext ctxt;
  innit_ctx(&ctxt, ctx_config, &program_allocator, "ESP BOY");
  program_ctxt->logger->log_info("TAG", "Initialized ctxt");

  auto _ = spawn(spawner());
  program_ctxt->clock->setup();
  _ = program_ctxt->loop->run();
  safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
}

extern "C" void app_main(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  xTaskCreate(task1, "Test name", 8192, NULL, 10, NULL);
}
