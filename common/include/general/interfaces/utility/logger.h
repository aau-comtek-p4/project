#ifndef LOGGER_INTERFACE_H
#define LOGGER_INTERFACE_H

#include <cstdint>
#define LOG_WARNING_PREFIX "\033[33m"
#define LOG_DEBUG_PREFIX "\033[38;2;124;159;255m"
#define LOG_INFO_PREFIX "\033[37m"
#define LOG_ERROR_PREFIX "\033[38;2;255;21;60m"

#define LOG_WARNING_TEXT "WARNING"
#define LOG_DEBUG_TEXT "DEBUG"
#define LOG_INFO_TEXT "INFO"
#define LOG_ERROR_TEXT "ERROR"

#define DEBUG_ON 1
enum LogLevel {
  LOG_INFO,
  LOG_DEBUG,
  LOG_ERROR,
  LOG_WARNING,
};

enum Component {};
enum Reason {};
union LogPayload {};

struct LogEntry {
  char msg[30];
  LogLevel level;
  Component component;
  Reason reason;
  uint64_t timestamp;
  uint64_t tick_count;
  LogPayload payload;
};

class LoggerInterface {
public:
  virtual __attribute__((format(printf, 3, 4))) void
  log_info(const char *tag, const char *fmt, ...) noexcept = 0;

  virtual __attribute__((format(printf, 3, 4))) void
  log_err(const char *tag, const char *fmt, ...) noexcept = 0;

  virtual __attribute__((format(printf, 3, 4))) void
  log_warning(const char *tag, const char *fmt, ...) noexcept = 0;

  virtual __attribute__((format(printf, 3, 4))) void
  log_debug(const char *tag, const char *fmt, ...) noexcept = 0;
};

#endif
