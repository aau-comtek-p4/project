#include "esp32/esp32_tasks/test_tasks.h"

#include "esp32/common.h"
#include "esp32/coroutines/misc/statistics.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context_innit.h"
#include "hal/uart_types.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
void uart_test_task(void *args) {
  device_id = 0;
  device_type = IODeviceType::DEVICE_SENSOR_NODE;

  ContextConfig<ESPContextSettings> ctx_config;
  ctx_config.clock_type = CtxtClockType::WALL;
  ctx_config.ctx_type = ContextType::NODE;
  ctx_config.logger_type = CtxtLoggerType::ESP_ASYNC_QUEUE_LOGGER;
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

  IOAddress address2 = {.addr_type = IOAddressType::FILE_DESCRIPTOR, .val = {}};
  address2.val.fd = UART_NUM_0;
  program_ctxt->connection_handler->add_connection(&address2);

  IOAddress address = {.addr_type = IOAddressType::FILE_DESCRIPTOR, .val = {}};
  address.val.fd = UART_NUM_1;
  program_ctxt->connection_handler->add_connection(&address);

  uint64_t free_memory = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
  program_ctxt->logger->log_entry(
      logging::log_debug("Free memory: %llu", free_memory));
  UBaseType_t remaining = uxTaskGetStackHighWaterMark(NULL);
  program_ctxt->logger->log_entry(
      logging::log_debug("Stack: %u", remaining * 4));

  auto _ = spawn(metric_printer());

  _ = program_ctxt->loop->run(60 * 1000);
  program_ctxt->logger->submit();
  program_ctxt->metrics->print_total_metrics();
  for (int i = 0; i < 10; i++) {
    program_ctxt->logger->log_entry(logging::log_debug("STOP"));
  }
  program_ctxt->logger->submit();
  printf("we good\n");
  while (true) {
    vTaskDelay(1);
  }
}
