#include "general/interfaces/utility/loggers/dummy_logger.h"

__attribute__((format(printf, 3, 4))) void
DummyLogger::log_info(const char *tag, const char *fmt, ...) noexcept {};

__attribute__((format(printf, 3, 4))) void
DummyLogger::log_debug(const char *tag, const char *fmt, ...) noexcept {};

__attribute__((format(printf, 3, 4))) void
DummyLogger::log_warning(const char *tag, const char *fmt, ...) noexcept {};

__attribute__((format(printf, 3, 4))) void
DummyLogger::log_err(const char *tag, const char *fmt, ...) noexcept {};
