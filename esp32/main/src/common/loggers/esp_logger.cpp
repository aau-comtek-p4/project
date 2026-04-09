#include "esp32/common/loggers/esp_logger.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include "esp_log_write.h"
#include "general/common.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
uint64_t ESPLogger::print_tag(const char *prefix, const char *type_text,
                              const char *tag, char *out_buf,
                              uint64_t max_size) {

  uint64_t timestamp = program_ctxt->clock->rt_since_start_ms();
  size_t pre_size =
      snprintf(out_buf, max_size, "%s[%08" PRIu64 "] [%s] [%s]: ", prefix,
               timestamp, tag, type_text);
  return pre_size;
}

void ESPLogger::log_general_v(const char *prefix, const char *type_text,
                              const char *tag, FILE *out_file, const char *fmt,
                              va_list args) noexcept {
  va_list copy;
  va_copy(copy, args);
  char out_buffer[256]; // fixed size, no VLA, no stack probe

  uint64_t timestamp = program_ctxt->clock->rt_since_start_ms();

  // Single pass — format everything at once
  int total_size = 0;
  int pos = snprintf(out_buffer, sizeof(out_buffer),
                     "%s[%08" PRIu64 "] [%s] [%s]: ", prefix, timestamp, tag,
                     type_text);
  total_size += pos;

  if (pos < (int)sizeof(out_buffer) - 1) {
    total_size +=
        vsnprintf(out_buffer + pos, sizeof(out_buffer) - pos, fmt, args);
  }

  out_buffer[total_size] = '\033';
  out_buffer[total_size + 1] = '[';
  out_buffer[total_size + 2] = '0';
  out_buffer[total_size + 3] = 'm';

  out_buffer[total_size + 4] = '\n';
  out_buffer[total_size + 5] = 0;

  // fwrite(out_buffer, sizeof(out_buffer[0]), total_size + 5, out_file);
  esp_log_write(ESP_LOG_INFO, tag, "%s", out_buffer);
}

__attribute__((format(printf, 3, 4))) void
ESPLogger::log_info(const char *tag, const char *fmt, ...) noexcept {
  va_list args;
  va_start(args, fmt);
  this->log_general_v(LOG_INFO_PREFIX, LOG_INFO_TEXT, tag, stdout, fmt, args);
  va_end(args);
}

__attribute__((format(printf, 3, 4))) void
ESPLogger::log_debug(const char *tag, const char *fmt, ...) noexcept {
  if (!DEBUG_ON) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  this->log_general_v(LOG_DEBUG_PREFIX, LOG_DEBUG_TEXT, tag, stdout, fmt, args);
  va_end(args);
}

__attribute__((format(printf, 3, 4))) void
ESPLogger::log_warning(const char *tag, const char *fmt, ...) noexcept {
  va_list args;
  va_start(args, fmt);
  this->log_general_v(LOG_WARNING_PREFIX, LOG_WARNING_TEXT, tag, stdout, fmt,
                      args);
  va_end(args);
}

__attribute__((format(printf, 3, 4))) void
ESPLogger::log_err(const char *tag, const char *fmt, ...) noexcept {
  va_list args;
  va_start(args, fmt);
  this->log_general_v(LOG_ERROR_PREFIX, LOG_ERROR_TEXT, tag, stderr, fmt, args);
  va_end(args);
}
