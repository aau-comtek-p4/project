
#include "cc.h"
#include "esp32/esp32_tasks/test_tasks.h"

#include "esp32/common.h"
#include "esp32/coroutines/misc/statistics.h"
#include "esp32/io/intializers/wifi_innit.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/interfaces/io/transports/tcp_transport.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context_innit.h"
#include "general/misc/crc.h"
#include "general/misc/shutdown.h"
#include "hal/uart_types.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
Job uart_reader_job() {

  auto transport =
      program_ctxt->io->get_transport<StorageIOTransport>(IOMethod::IO_SERIAL);
  IOAddress uart_addr;
  uart_addr.addr_type = IOAddressType::FILE_DESCRIPTOR;
  uart_addr.val.fd = UART_NUM_0;
  auto buf = (uint8_t *)program_ctxt->buffer_allocator->allocate(1024).value();
  uint64_t bytes_in_buf = 0;
  uint64_t last_valid = 0;
  IOConnection *connection =
      program_ctxt->connection_handler->get_or_add_connection(&uart_addr);
  while (true) {
    auto res = co_await transport->io_read(&uart_addr, buf + bytes_in_buf,
                                           1024 - bytes_in_buf);
    if (!res.has_value()) {
      safe_shutdown(res.error());
    }
    bytes_in_buf += res.value();
    for (uint64_t i = 0; i + sizeof(IOUARTFrameHeader) <= bytes_in_buf; i++) {
      auto uart_frame_header = (IOUARTFrameHeader *)(buf + i);
      if (uart_frame_header->magic != UART_MAGIC_HEADER) {
        continue;
      }
      if (uart_frame_header->package_header.package_type !=
          IOPackageType::IOPackage_ACK) {
        continue;
      }
      uint64_t package_size = 0;
      if (i + sizeof(IOUARTFrameHeader) + package_size + sizeof(uint32_t) >
          bytes_in_buf) {
        break;
      }

      uint32_t crc = CRC32(&buf[i], sizeof(IOUARTFrameHeader) + package_size);
      auto crc_ptr =
          (uint32_t *)(buf + i + sizeof(IOUARTFrameHeader) + package_size);
      if (crc != *crc_ptr) {
        continue;
      }
      connection->ack.state = ACK_ACKNOWLEGDED;

      last_valid =
          sizeof(IOUARTFrameHeader) + package_size + sizeof(uint32_t) + i;
      i = last_valid - 1;
    }
    bytes_in_buf -= last_valid;
    memmove(buf, buf + last_valid, bytes_in_buf);
  }
}

void tcp_test_task(void *args) {
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
  const char *wifi_ssid = "Redmi 14C";
  const char *password = "ufgf7711";
  // initialize_wifi_station((uint8_t *)wifi_ssid, (uint8_t *)password);

  uint64_t free_memory = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);

  program_ctxt->logger->log_entry(logging::log_debug(
      "Program size: %llu", total_allocator.amount_allocated));
  program_ctxt->logger->log_entry(
      logging::log_debug("Free memory: %llu", free_memory));
  UBaseType_t remaining = uxTaskGetStackHighWaterMark(NULL);
  program_ctxt->logger->log_entry(
      logging::log_debug("Stack: %u", remaining * 4));

  // auto _ = spawn(tcp_server());
  auto _ = spawn(metric_printer());
  spawn(uart_reader_job()).value();

  program_ctxt->loop->run().value();
  safe_shutdown(ErrorWrapper{.error = -1, .tag = ErrorWrapper::CUSTOM});
}
