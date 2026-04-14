#ifndef FWRITE_LOGGER_H
#define FWRITE_LOGGER_H

#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>

class FWriteLogger : public LoggerInterface {
private:
  char log_buf[MAX_LOG_SIZE] = {0};
  LogSerializerInterface *serializer;

public:
  FWriteLogger(LogSerializerInterface *serializer);
  void log_entry(LogEntry log_entry) noexcept override;

  void submit(uint64_t timeout) noexcept override;
  void submit() noexcept override;
};

#endif
