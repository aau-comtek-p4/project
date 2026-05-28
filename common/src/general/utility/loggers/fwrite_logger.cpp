#include "general/interfaces/utility/loggers/fwrite_logger.h"
#include "general/common.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include <cinttypes>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>

FWriteLogger::FWriteLogger(LogSerializerInterface *serializer) {
  this->serializer = serializer;
}
void FWriteLogger::log_entry(LogEntry log_entry) noexcept {
  log_entry.log_count = this->log_count;
  this->log_count += 1;
  uint64_t bytes_written =
      this->serializer->serialize(this->log_buf, MAX_LOG_SIZE, log_entry);

  this->log_buf[bytes_written] = 0;

  fwrite(this->log_buf, sizeof(this->log_buf[0]), bytes_written, stderr);
};

void FWriteLogger::submit(uint64_t timeout) noexcept {};
void FWriteLogger::submit() noexcept {};
