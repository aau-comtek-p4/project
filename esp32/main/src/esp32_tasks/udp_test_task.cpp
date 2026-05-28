#include "esp32/common.h"
#include "esp32/common/context.h"
#include "esp32/coroutines/misc/statistics.h"
#include "esp32/esp32_tasks/test_tasks.h"
#include "esp32/io/intializers/wifi_innit.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/misc/context_innit.h"
#include <cstdio>

void udp_test_task(void *args) {
  while (!esp_started) {
    vTaskDelay(1);
  }
  vTaskDelay(pdMS_TO_TICKS(1000));
  ContextConfig<ESPContextSettings> ctx_config;
  ctx_config.clock_type = CtxtClockType::WALL;
  ctx_config.ctx_type = ContextType::NODE;
  ctx_config.logger_type = CtxtLoggerType::FWRITE_LOGGER;
  ctx_config.log_serializer_type = CtxtLoggerSerializer::JSON_SERIALIZER;
  ctx_config.random_type = CtxtRandomType::NONE;
  ctx_config.deadline_tracker_type = CtxtDeadlineTrackerType::MIN_HEAP;
  ProgramContext ctxt;
  uint8_t *total_buffer =
      (uint8_t *)calloc(ctx_config.settings.max_total_size, sizeof(uint8_t));
  if (!total_buffer) {
    fprintf(stderr, "Could not allocate buffer\n");
    return;
  }
  ArenaAllocator total_allocator(NAME_PROGRAM_ALLOCATOR, total_buffer,
                                 ctx_config.settings.max_total_size);
  program_ctxt = &ctxt;
  innit_ctx(&ctxt, ctx_config, &total_allocator);

  initialize_wifi_station((uint8_t *)CONFIG_ESP_WIFI_SSID,
                          (uint8_t *)CONFIG_ESP_WIFI_PASSWORD);
  if (!wifi_initialized) {
    program_ctxt->logger->log_entry(logging::log_debug("WIFI failed"));
    safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
  }
  program_ctxt->logger->log_entry(logging::log_debug("WIFI connected"));
  uint64_t free_memory = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
  program_ctxt->logger->log_entry(
      logging::log_debug("Free memory: %llu", free_memory));
  UBaseType_t remaining = uxTaskGetStackHighWaterMark(NULL);
  program_ctxt->logger->log_entry(
      logging::log_debug("Stack: %u", remaining * 4));

  auto _ = spawn(metric_printer());
  /*
  _ = spawn(udp_test_server());
  _ = spawn(udp_test_sender());
  */

  _ = program_ctxt->loop->run(0);
  program_ctxt->logger->submit();
  program_ctxt->metrics->print_metrics();
  for (int i = 0; i < 10; i++) {
    program_ctxt->logger->log_entry(logging::log_debug("STOP"));
  }
  program_ctxt->logger->submit();
  while (true) {
    vTaskDelay(1);
  }
};
