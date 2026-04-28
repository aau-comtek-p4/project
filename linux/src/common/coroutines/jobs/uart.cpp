
#include "common/io/io.h"
#include "general/awaiters/sleep_for.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/misc/crc.h"
#include <iostream>
Job read_uart() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_END + 7);
  self_ctxt->trace.start();
  const char *port = "/dev/ttyACM0";
  const char *esp_log = "esp_log-" CONFIG_PROJECT_ID ".txt";
  auto serial_transport =
      program_ctxt->io->get_transport<StorageIOTransport>(IOMethod::IO_SERIAL);
  auto file_transport =
      program_ctxt->io->get_transport<StorageIOTransport>(IOMethod::IO_FILE);

  IOAddress esp_addr{.addr_type = IOAddress::FILE_PATH};
  strncpy(esp_addr.file_path, esp_log, 30);
  IOAddress port_addr{.addr_type = IOAddress::FILE_PATH};
  strncpy(port_addr.file_path, port, 30);

  auto res = co_await file_transport->io_open(esp_addr);
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
  int esp_log_fd = res.value();

  res = co_await serial_transport->io_open(port_addr);
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
  int fd = res.value();
  IOAddress esp_log_fd_fd{.addr_type = IOAddress::FILE_DESCRIPTOR,
                          .fd = esp_log_fd};
  IOAddress port_fd{.addr_type = IOAddress::FILE_DESCRIPTOR, .fd = fd};
  uint8_t *buf =
      (uint8_t *)program_ctxt->buffer_allocator->allocate(1024).value();
  uint8_t *temp_buf =
      (uint8_t *)program_ctxt->buffer_allocator->allocate(1024).value();

  JsonLogSerializer serializer;
  int n;
  int len = 0;
  UARTLogFrame *frame;
  uint32_t check = 0;
  uint64_t missecd_check_count = 0;
  uint64_t total_frame_count = 0;
  const char *stop_msg = "STOP";
  uint64_t last_count = 0;
  uint64_t missed_frame = 0;
  int i = 0;
  uint32_t magic_candidate;
  while (program_ctxt->loop->running) {
    res = co_await serial_transport->io_read(port_fd, buf + len, 1024 - len);
    if (!res.has_value()) {
      safe_shutdown(res.error());
    }
    n = res.value();
    if (n <= 0) {
      break;
    }
    len += n;
    i = 0;
    while (i <= len - (int)sizeof(UARTLogFrame)) {
      if (i + (int)sizeof(UARTLogFrame) > len) {
        break; // wait for more data
      }
      memcpy(&magic_candidate, &buf[i], sizeof(uint32_t));

      if (magic_candidate != UART_MAGIC_HEADER) {
        i++;
        continue;
      }
      frame = (UARTLogFrame *)&buf[i];
      if (frame->magic != UART_MAGIC_HEADER) {
        i++;
        continue;
      }
      total_frame_count += 1;
      check = CRC32((uint8_t *)frame, sizeof(UARTLogFrame) - sizeof(uint32_t));
      if (check != frame->check) {
        missecd_check_count += 1;
        i++;
        continue;
      }
      if (last_count != frame->entry.log_count) {
        if (last_count != 0) {
          std::cerr << "Missed frame: " << last_count << "\n";
          for (uint64_t i = last_count; i < frame->entry.log_count; i++) {
            missed_frame += 1;
          }
          last_count = frame->entry.log_count;
        } else {
          last_count = frame->entry.log_count;
        }
      }
      last_count++;
      check = 0;
      uint64_t bytes_written =
          serializer.serialize((char *)temp_buf, 1024, frame->entry);
      res = co_await file_transport->io_write(esp_log_fd_fd, temp_buf,
                                              bytes_written);
      if (!res.has_value()) {
        safe_shutdown(res.error());
      }
      if (frame->entry.reason == LogReason::REASON_DEBUG) {
        if (!strncmp(frame->entry.payload.debug.debug, stop_msg,
                     strlen(stop_msg))) {
          printf("Got stop\n");
          printf("Missed check: %lu\n", missecd_check_count);
          printf("Total frames: %lu\n", total_frame_count);
          printf("Missed fames: %lu\n", missed_frame);
          program_ctxt->logger->log_entry(logging::log_debug("Got stop msg"));
          program_ctxt->loop->stop();
          break;
        }
      }
      i += sizeof(UARTLogFrame);
    }
    memmove(buf, buf + i, len - i);
    len -= i;
  }

  res = co_await serial_transport->io_close(esp_log_fd_fd);
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
  res = co_await serial_transport->io_close(port_fd);
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}

Job uart_writer() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_END + 7);
  self_ctxt->trace.start();
  const char *port = "/dev/ttyACM0";
  auto serial_transport =
      program_ctxt->io->get_transport<StorageIOTransport>(IOMethod::IO_SERIAL);
  IOAddress port_addr{.addr_type = IOAddress::FILE_PATH};
  strncpy(port_addr.file_path, port, 30);
  auto res = co_await serial_transport->io_open(port_addr);
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
  uint8_t out_byte = 'Z';

  uint64_t count = 0;
  IOAddress fd_addr{.addr_type = IOAddress::FILE_DESCRIPTOR, .fd = res.value()};
  auto _ =
      co_await serial_transport->io_write(fd_addr, &out_byte, sizeof(out_byte));
  UARTLogFrame frame{.magic = UART_MAGIC_HEADER,
                     .pad1 = 0,
                     .entry = logging::log_debug("Hello, LINUX%lu", count),
                     .pad2 = 0};
  frame.check = CRC32((uint8_t *)&frame, sizeof(frame) - sizeof(uint32_t));

  while (program_ctxt->loop->running) {
    auto write_res = co_await serial_transport->io_write(
        fd_addr, (uint8_t *)&frame, sizeof(frame));
    if (!write_res.has_value()) {
      printf("errno: %i\n", write_res.error().error);
      safe_shutdown(write_res.error());
    }
    count += 1;
    frame.magic = UART_MAGIC_HEADER;
    frame.pad1 = 0;
    frame.entry = logging::log_debug("Hello, LINUX%lu", count);
    frame.pad2 = 0;
    frame.check = CRC32((uint8_t *)&frame, sizeof(frame) - sizeof(uint32_t));
    co_await sleep_for(100);
  }
}
