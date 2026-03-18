#ifndef LOGGER_INTERFACE_H
#define LOGGER_INTERFACE_H

class LoggerInterface {
public:
  virtual __attribute__((format(printf, 3, 4))) void
  log(const char *tag, const char *fmt, ...) noexcept = 0;

  virtual __attribute__((format(printf, 3, 4))) void
  log_err(const char *tag, const char *fmt, ...) noexcept = 0;
};

#endif
