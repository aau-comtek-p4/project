#ifndef DUMMY_LOGGER_H
#define DUMMY_LOGGER_H
#include "general/interfaces/utility/logger.h"
class DummyLogger : public LoggerInterface {
  __attribute__((format(printf, 3, 4))) void
  log_info(const char *tag, const char *fmt, ...) noexcept override;

  __attribute__((format(printf, 3, 4))) void
  log_err(const char *tag, const char *fmt, ...) noexcept override;

  __attribute__((format(printf, 3, 4))) void
  log_warning(const char *tag, const char *fmt, ...) noexcept override;

  __attribute__((format(printf, 3, 4))) void
  log_debug(const char *tag, const char *fmt, ...) noexcept override;
};

#endif
