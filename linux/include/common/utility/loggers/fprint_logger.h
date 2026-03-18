#ifndef FPRINTF_LOGGER_H
#define FPRINTF_LOGGER_H

#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include <cstddef>

class FPrintLogger : public LoggerInterface {
private:
  ClockInterface *clock;

public:
  FPrintLogger(ClockInterface *clock);
  __attribute__((format(printf, 3, 4))) void
  log(const char *tag, const char *format, ...) noexcept override;

  __attribute__((format(printf, 3, 4))) void
  log_err(const char *tag, const char *format, ...) noexcept override;
};

#endif
