#ifndef DUMMY_LOGGER_H
#define DUMMY_LOGGER_H
#include "general/interfaces/utility/logger.h"
#include <cstdint>
class DummyLogger : public LoggerInterface {
public:
  void log_entry(LogEntry entry) noexcept override;
  void submit() noexcept override;
  void submit(uint64_t timeout) noexcept override;
};

#endif
