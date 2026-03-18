#include "common/utility/loggers/fprint_logger.h"
#include "general/interfaces/utility/clock.h"
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <format>
#include <unistd.h>

FPrintLogger::FPrintLogger(ClockInterface *clock) : clock(clock) {}

__attribute__((format(printf, 3, 4))) void
FPrintLogger::log(const char *tag, const char *fmt, ...) noexcept {
  va_list args, copy;
  va_start(args, fmt);
  va_copy(copy, args);

  auto time_stamp = this->clock->format_time();
  size_t time_stamp_size =
      snprintf(nullptr, 0, "[%02lu:%02lu:%02lu] ", time_stamp.time_h,
               time_stamp.time_m, time_stamp.time_s) +
      1;
  size_t tag_size = snprintf(nullptr, 0, "[%s]: ", tag) + 1;

  size_t str_size = vsnprintf(nullptr, 0, fmt, copy) + 1;
  size_t total_size = time_stamp_size + tag_size + str_size;
  char out_buffer[total_size + 1];
  snprintf(out_buffer, time_stamp_size, "[%02lu:%02lu:%02lu] ",
           time_stamp.time_h, time_stamp.time_m, time_stamp.time_s);

  snprintf(out_buffer + time_stamp_size, tag_size, "[%s]: ", tag);
  vsnprintf(out_buffer + time_stamp_size + tag_size, str_size, fmt, args);
  out_buffer[total_size] = '\n';

  fwrite(out_buffer, sizeof(out_buffer[0]), total_size + 1, stdout);

  va_end(copy);
  va_end(args);
}

__attribute__((format(printf, 3, 4))) void
FPrintLogger::log_err(const char *tag, const char *fmt, ...) noexcept {
  va_list args, copy;
  va_start(args, fmt);
  va_copy(copy, args);

  auto time_stamp = this->clock->format_time();
  size_t time_stamp_size =
      snprintf(nullptr, 0, "\033[31m[%02lu:%02lu:%02lu] ", time_stamp.time_h,
               time_stamp.time_m, time_stamp.time_s) +
      1;
  size_t tag_size = snprintf(nullptr, 0, "[%s]: ", tag) + 1;

  size_t str_size = vsnprintf(nullptr, 0, fmt, copy) + 1;
  size_t total_size = time_stamp_size + tag_size + str_size;
  char out_buffer[total_size + 5];
  snprintf(out_buffer, time_stamp_size, "\033[31m[%02lu:%02lu:%02lu] ",
           time_stamp.time_h, time_stamp.time_m, time_stamp.time_s);

  snprintf(out_buffer + time_stamp_size, tag_size, "[%s]: ", tag);
  vsnprintf(out_buffer + time_stamp_size + tag_size, str_size, fmt, args);
  out_buffer[total_size] = '\033';
  out_buffer[total_size + 1] = '[';
  out_buffer[total_size + 2] = '0';
  out_buffer[total_size + 3] = 'm';
  out_buffer[total_size + 4] = '\n';

  fwrite(out_buffer, sizeof(out_buffer[0]), total_size + 5, stderr);

  va_end(copy);
  va_end(args);
}
