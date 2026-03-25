#include "common/utility/loggers/fprint_logger.h"
#include "general/common.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <unistd.h>

size_t FPrintLogger::print_tag(const char *prefix, const char *type_text,
                               const char *tag, char *out_buf,
                               size_t max_size) {

  auto time_stamp = program_ctxt->clock->format_time();
  size_t pre_size = snprintf(out_buf, max_size, "%s[%08lu] [%s] [%s]: ", prefix,
                             time_stamp.time_ms, tag, type_text) +
                    1;
  return pre_size;
}

void FPrintLogger::log_general_v(const char *prefix, const char *type_text,
                                 const char *tag, FILE *out_file,
                                 const char *fmt, va_list args) noexcept {
  va_list copy;
  va_copy(copy, args);

  size_t pre_size = this->print_tag(prefix, type_text, tag, nullptr, 0);
  size_t str_size = vsnprintf(nullptr, 0, fmt, copy) + 1;
  va_end(copy);
  size_t total_size = pre_size + str_size;
  char out_buffer[total_size + 5];
  this->print_tag(prefix, type_text, tag, out_buffer, pre_size);
  vsnprintf(out_buffer + pre_size, str_size, fmt, args);
  out_buffer[total_size] = '\033';
  out_buffer[total_size + 1] = '[';
  out_buffer[total_size + 2] = '0';
  out_buffer[total_size + 3] = 'm';

  out_buffer[total_size + 4] = '\n';

  fwrite(out_buffer, sizeof(out_buffer[0]), total_size + 5, out_file);
}

__attribute__((format(printf, 3, 4))) void
FPrintLogger::log_info(const char *tag, const char *fmt, ...) noexcept {
  va_list args;
  va_start(args, fmt);
  this->log_general_v(LOG_INFO_PREFIX, LOG_INFO_TEXT, tag, stdout, fmt, args);
  va_end(args);
}

__attribute__((format(printf, 3, 4))) void
FPrintLogger::log_debug(const char *tag, const char *fmt, ...) noexcept {
  if (!DEBUG_ON) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  this->log_general_v(LOG_DEBUG_PREFIX, LOG_DEBUG_TEXT, tag, stdout, fmt, args);
  va_end(args);
}

__attribute__((format(printf, 3, 4))) void
FPrintLogger::log_warning(const char *tag, const char *fmt, ...) noexcept {
  va_list args;
  va_start(args, fmt);
  this->log_general_v(LOG_WARNING_PREFIX, LOG_WARNING_TEXT, tag, stdout, fmt,
                      args);
  va_end(args);
}

__attribute__((format(printf, 3, 4))) void
FPrintLogger::log_err(const char *tag, const char *fmt, ...) noexcept {
  va_list args;
  va_start(args, fmt);
  this->log_general_v(LOG_ERROR_PREFIX, LOG_ERROR_TEXT, tag, stderr, fmt, args);
  va_end(args);
}
