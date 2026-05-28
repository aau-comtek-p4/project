#include "cc.h"
#include "esp32/esp32_tasks/test_tasks.h"

#include "esp32/common.h"
#include "esp32/coroutines/misc/statistics.h"
#include "esp32/io/intializers/wifi_innit.h"
#include "esp32/io/io.h"
#include "esp_now.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "general/awaiters/sleep_for.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/interfaces/io/transports/tcp_transport.h"
#include "general/interfaces/io/transports/udp_transport.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context_innit.h"
#include "general/misc/crc.h"
#include "general/misc/shutdown.h"
#include "hal/uart_types.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include <cerrno>
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

Job esp_now_sender() {
  auto transport =
      program_ctxt->io->get_transport<UDPIOTransport>(IOMethod::IO_ESP_NOW);

  IOAddress cli_addr;
  uint8_t cli_mac[6] = {0xd4, 0xd4, 0xda, 0x5a, 0xc1, 0x80};
  cli_addr.addr_type = IOAddressType::MAC;
  memcpy(cli_addr.val.sockaddr, cli_mac, ESP_NOW_ETH_ALEN);

  while (true) {
    uint64_t start_time = program_ctxt->clock->rt_since_start_ns();
    auto res = co_await transport->send_to(
        nullptr, &cli_addr, 0, 0, IOPackageType::IOPackage_NULL, false);
    if (!res.has_value()) {
      safe_shutdown(res.error());
    }
    uint64_t end_time = program_ctxt->clock->rt_since_start_ns();
    program_ctxt->logger->log_entry(
        logging::log_debug("RTT: %" PRIu64, end_time - start_time));
  }

  co_return;
}

Job esp_now_basic_reciver() {
  auto transport =
      program_ctxt->io->get_transport<UDPIOTransport>(IOMethod::IO_ESP_NOW);

  IOAddress cli_addr;
  cli_addr.addr_type = IOAddressType::MAC;
  auto buf = (uint8_t *)program_ctxt->buffer_allocator->allocate(1024).value();

  while (true) {
    auto res = co_await transport->recv_from(nullptr, &cli_addr, buf, 1024);
    if (!res.has_value()) {
      safe_shutdown(res.error());
    }
    res = co_await transport->send_to(nullptr, &cli_addr, 0, 0,
                                      IOPackageType::IOPackage_NULL, false);
    if (!res.has_value()) {
      safe_shutdown(res.error());
    }
  }
  co_return;
}
bool is_server = false;

void espnow_test_task(void *args) {
  device_id = 10;
  device_type = IODeviceType::DEVICE_SENSOR_NODE;

  ContextConfig<ESPContextSettings> ctx_config;
  ctx_config.clock_type = CtxtClockType::WALL;
  ctx_config.ctx_type = ContextType::NODE;
  if (is_server) {
    ctx_config.logger_type = CtxtLoggerType::FWRITE_LOGGER;
  } else {
    ctx_config.logger_type = CtxtLoggerType::ESP_ASYNC_QUEUE_LOGGER;
  }
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
  initialize_wifi_station(nullptr, nullptr);
  program_ctxt->logger->log_entry(logging::log_debug("ESP WIFI STA started"));
  self_esp_now_innit();

  uint64_t free_memory = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);

  program_ctxt->logger->log_entry(logging::log_debug(
      "Program size: %llu", total_allocator.amount_allocated));
  program_ctxt->logger->log_entry(
      logging::log_debug("Free memory: %llu", free_memory));
  UBaseType_t remaining = uxTaskGetStackHighWaterMark(NULL);
  program_ctxt->logger->log_entry(
      logging::log_debug("Stack: %u", remaining * 4));

  // auto _ = spawn(tcp_server());
  //
  // spawn(esp_now_handler()).value();
  //
  if (is_server) {
    // spawn(uart_reader_job()).value();
    spawn(esp_now_basic_reciver()).value();
    // spawn(metric_printer()).value();
    program_ctxt->loop->run().value();
  } else {
    // spawn(espnow_receiver_routine()).value();
    //
    spawn(uart_reader_job()).value();
    spawn(metric_printer()).value();
    spawn(esp_now_sender()).value();
    program_ctxt->loop->run(60000).value();
  }

  safe_shutdown(ErrorWrapper{.error = -1, .tag = ErrorWrapper::CUSTOM});
}
