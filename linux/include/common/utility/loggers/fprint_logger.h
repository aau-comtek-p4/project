#ifndef FPRINTF_LOGGER_H
#define FPRINTF_LOGGER_H

#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include <cstdarg>
#include <cstddef>
#include <cstdio>

class FPrintLogger : public LoggerInterface {
private:
  size_t print_tag(const char *prefix, const char *type_text, const char *tag,
                   char *out_buf, size_t max_size);

  void log_general_v(const char *prefix, const char *type_text, const char *tag,
                     FILE *out_f, const char *format, va_list args) noexcept;

public:
  __attribute__((format(printf, 3, 4))) void
  log_info(const char *tag, const char *format, ...) noexcept override;

  __attribute__((format(printf, 3, 4))) void
  log_err(const char *tag, const char *format, ...) noexcept override;

  __attribute__((format(printf, 3, 4))) void
  log_warning(const char *tag, const char *format, ...) noexcept override;

  __attribute__((format(printf, 3, 4))) void
  log_debug(const char *tag, const char *format, ...) noexcept override;
};

#endif
