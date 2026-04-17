#include "common/logger/file_logger.h"
#include "common/io/io.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
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
FileLogger::FileLogger(QueueInterface<LogEntry> *msg_queue,
                       LogSerializerInterface *serializer)
    : msg_queue(msg_queue), serializer(serializer) {
  this->file_descriptor = open(LOG_FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0644);
}

void FileLogger::log_entry(LogEntry entry) noexcept {
  // auto res = this->msg_queue.enque(std::move(entry));
  auto res = this->msg_queue->enque(std::move(entry));
  if (res.has_value()) {
    return;
  }
  if (entry.severity == LogLevel::LOG_ERROR) {
    auto _ = this->msg_queue->deque();
    res = this->msg_queue->enque(std::move(entry));
  }
  this->dropped_count += 1;
};

void FileLogger::submit() noexcept {
  auto log_msg = this->msg_queue->deque();
  uint64_t current_time = program_ctxt->clock->rt_since_start_ns();
  uint64_t last_time = current_time;
  while (log_msg.has_value()) {
    uint64_t bytes_written = this->serializer->serialize(
        this->out_buf, MAX_LOG_SIZE, log_msg.value());
    write(this->file_descriptor, this->out_buf, bytes_written);
    log_msg = this->msg_queue->deque();

    current_time = program_ctxt->clock->rt_since_start_ns();
    uint64_t time_taken = current_time - last_time;
    last_time = current_time;
    program_ctxt->metrics->document_statistics_metric_metric(
        StatMetricType::METRIC_LOG_TIME, time_taken);
  }
};

void FileLogger::submit(uint64_t timeout) noexcept {
  uint64_t start_time = program_ctxt->clock->rt_since_start_ns();
  uint64_t last_write_time = start_time;
  uint64_t current_time = start_time;
  uint64_t worst_write_time = 0;
  uint64_t time_taken = 0;
  auto log_msg = this->msg_queue->deque();
  while (log_msg.has_value() &&
         (current_time - start_time + worst_write_time) < timeout) {
    uint64_t bytes_written = this->serializer->serialize(
        this->out_buf, MAX_LOG_SIZE, log_msg.value());
    write(this->file_descriptor, this->out_buf, bytes_written);
    current_time = program_ctxt->clock->rt_since_start_ns();
    time_taken = current_time - last_write_time;
    program_ctxt->metrics->document_statistics_metric_metric(
        StatMetricType::METRIC_LOG_TIME, time_taken);
    if (time_taken > worst_write_time) {
      worst_write_time = time_taken;
    }
    last_write_time = current_time;
    log_msg = this->msg_queue->deque();
  }
  if (this->dropped_count > 0) {
    program_ctxt->metrics->document_statistics_metric_metric(
        StatMetricType::METRIC_LOG_DROP, dropped_count);
    LogEntry entry = logging::log_dropped_logs(this->dropped_count);
    uint64_t bytes_written =
        this->serializer->serialize(this->out_buf, MAX_LOG_SIZE, entry);
    write(this->file_descriptor, this->out_buf, bytes_written);
    this->dropped_count = 0;
  }
};
