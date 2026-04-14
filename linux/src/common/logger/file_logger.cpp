#include "common/logger/file_logger.h"
#include "common/io/io.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/names.h"
#include "general/misc/shutdown.h"
#include <cinttypes>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <utility>
FileLogger::FileLogger(LogSerializerInterface *serializer)
    : msg_queue(NAME_LOGGING_QUEUE), serializer(serializer) {
  this->file_descriptor = open(LOG_FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0644);
}

void FileLogger::log_entry(LogEntry entry) noexcept {
  auto _ = this->msg_queue.enque(std::move(entry));
};

void FileLogger::submit() noexcept {
  auto log_msg = this->msg_queue.deque();
  while (log_msg.has_value()) {
    uint64_t bytes_written = this->serializer->serialize(
        this->out_buf, MAX_LOG_SIZE, log_msg.value());
    write(this->file_descriptor, this->out_buf, bytes_written);
    log_msg = this->msg_queue.deque();
  }
};

void FileLogger::submit(uint64_t timeout) noexcept {
  uint64_t start_time = program_ctxt->clock->rt_since_start_ns();
  uint64_t last_write_time = start_time;
  uint64_t current_time = start_time;
  uint64_t worst_write_time = 0;
  uint64_t time_taken = 0;
  auto log_msg = this->msg_queue.deque();
  while (log_msg.has_value() &&
         (current_time - start_time + worst_write_time) < timeout) {
    uint64_t bytes_written = this->serializer->serialize(
        this->out_buf, MAX_LOG_SIZE, log_msg.value());
    write(this->file_descriptor, this->out_buf, bytes_written);
    current_time = program_ctxt->clock->rt_since_start_ns();
    time_taken = current_time - last_write_time;
    if (time_taken > worst_write_time) {
      worst_write_time = time_taken;
    }
    log_msg = this->msg_queue.deque();
  }
};
