#include "common.h"
#include "common/context.h"
#include "common/coroutines/misc/statistics.h"
#include "common/io/second_process_io.h"
#include "general/awaiters/sleep_for.h"
#include "general/awaiters/yield_awaiter.h"
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/generator.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/interfaces/io/transports/tcp_transport.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/storage/allocators/bucket_allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/context.h"
#include "general/misc/context_innit.h"
#include "general/misc/crc.h"
#include "general/misc/errors.h"
#include "general/misc/names.h"
#include "general/misc/shutdown.h"
#include <arpa/inet.h>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <liburing.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <thread>
#include <type_traits>
#include <unistd.h>
#define ESP_LOG_NAME "esp_log.txt"

Task<int> handle_uart(IOAddress *esp_log_fd, IOAddress *uart_serial_fd) {

  auto serial_transport =
      program_ctxt->io->get_transport<StorageIOTransport>(IOMethod::IO_SERIAL);
  auto file_transport =
      program_ctxt->io->get_transport<StorageIOTransport>(IOMethod::IO_FILE);

  uint64_t bytes_in_buf = 0;
  uint64_t last_valid = 0;
  auto buf = (uint8_t *)program_ctxt->buffer_allocator->allocate(1024).value();
  auto out_buf =
      (uint8_t *)program_ctxt->buffer_allocator->allocate(1024).value();

  LogEntry temp_entry;
  JsonLogSerializer serializer;
  while (true) {
    if (bytes_in_buf >= 1024) {
      printf("Bytes in buf surpasses buf size %lu\n", bytes_in_buf);
      safe_shutdown(ErrorWrapper{
          .error = 1,
          .tag = ErrorWrapper::CUSTOM,
      });
    }
    auto res = co_await serial_transport->io_read(
        uart_serial_fd, buf + bytes_in_buf, 1024 - bytes_in_buf);
    if (!res.has_value()) {
      printf("Read failed\n");
      safe_shutdown(res.error());
    }
    bytes_in_buf += res.value();
    last_valid = 0;
    for (uint64_t i = 0; i + sizeof(IOUARTFrameHeader) <= bytes_in_buf; i++) {
      auto uart_frame_header = (IOUARTFrameHeader *)(buf + i);
      if (uart_frame_header->magic != UART_MAGIC_HEADER) {
        continue;
      }
      if (uart_frame_header->package_header.package_type !=
          IOPackageType::IOPackage_LOG) {
        continue;
      }
      uint64_t package_size = sizeof(LogEntry);
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
      auto log_entry = (LogEntry *)(buf + sizeof(IOUARTFrameHeader) + i);
      memcpy(&temp_entry, log_entry, package_size);
      uint64_t bytes_written =
          serializer.serialize((char *)out_buf, 1024, temp_entry);
      res = co_await file_transport->io_write(
          esp_log_fd, out_buf, bytes_written, IOPackageType::IOPackage_NULL);
      if (!res.has_value()) {
        safe_shutdown(res.error());
      }
      if (log_entry->reason == LogReason::REASON_DEBUG) {
        if (!strcmp(log_entry->payload.debug.debug, "STOP")) {
          safe_shutdown(ErrorWrapper{.error = -1, .tag = ErrorWrapper::CUSTOM});
        }
      }
      if (uart_frame_header->package_header.with_ack) {
        co_await serial_transport->io_write(uart_serial_fd, 0, 0,
                                            IOPackageType::IOPackage_ACK);
      }

      last_valid =
          sizeof(IOUARTFrameHeader) + package_size + sizeof(uint32_t) + i;
      i = last_valid - 1;
    }
    bytes_in_buf -= last_valid;
    memmove(buf, buf + last_valid, bytes_in_buf);
  }
  co_return 0;
}

const char *out_file = "esp_log.txt";
Job uart_test_reader() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(program_ctxt->name_lookup->append_name("uart_reader"));
  self_ctxt->trace.start();
  auto serial_transport =
      program_ctxt->io->get_transport<StorageIOTransport>(IOMethod::IO_SERIAL);
  auto file_transport =
      program_ctxt->io->get_transport<StorageIOTransport>(IOMethod::IO_FILE);
  IOAddress file_addr{.addr_type = IOAddressType::FILE_PATH};
  file_addr.val.file_name_index =
      program_ctxt->name_lookup->append_name(ESP_LOG_NAME);
  auto res = co_await file_transport->io_open(&file_addr);
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
  IOAddress file_addr_fd{.addr_type = IOAddressType::FILE_DESCRIPTOR};
  file_addr_fd.val.fd = res.value();

  IOAddress uart_serial_name{.addr_type = IOAddressType::FILE_PATH};
  uart_serial_name.val.file_name_index =
      program_ctxt->name_lookup->append_name("/dev/ttyACM2");
  res = co_await serial_transport->io_open(&uart_serial_name);
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
  IOAddress uart_serial_fd{.addr_type = IOAddressType::FILE_DESCRIPTOR};
  uart_serial_fd.val.fd = res.value();

  co_await handle_uart(&file_addr_fd, &uart_serial_fd);
}

int main() {
  device_id = 69;
  device_type = IODeviceType::DEVICE_CENTRAL_SERVER;
  ContextConfig<NodeContextSettings> ctx_config;
  ctx_config.clock_type = CtxtClockType::WALL;
  ctx_config.ctx_type = ContextType::NODE;
  ctx_config.logger_type = CtxtLoggerType::FWRITE_LOGGER;
  ctx_config.log_serializer_type = CtxtLoggerSerializer::JSON_SERIALIZER;
  ctx_config.random_type = CtxtRandomType::NONE;
  ctx_config.deadline_tracker_type = CtxtDeadlineTrackerType::MIN_HEAP;

  initialize_second_process_io(ctx_config.settings.max_total_size);
  if (second_process_ctxt.child_pid == 0) {
    std::signal(SIGINT, SIG_IGN);
    second_process_io_loop();
  }

  auto hande_sigint = [](int a) { program_ctxt->loop->stop(); };
  ArenaAllocator total_allocator(NAME_PROGRAM_ALLOCATOR,
                                 second_process_ctxt.shared_mem,
                                 ctx_config.settings.max_total_size);
  ProgramContext ctxt;
  std::signal(SIGINT, hande_sigint);
  innit_ctx(&ctxt, ctx_config, &total_allocator);
  memcpy(second_process_ctxt.program_ctxt, &ctxt, sizeof(ProgramContext));
  second_process_notify(second_process_ctxt.efd_start);

  // auto res = spawn(metric_logger());
  auto res = spawn(uart_test_reader());

  res = program_ctxt->loop->run();

  safe_shutdown(ErrorWrapper{.error = -1, .tag = ErrorWrapper::CUSTOM});
}
