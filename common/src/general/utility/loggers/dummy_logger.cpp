#include "general/interfaces/utility/loggers/dummy_logger.h"

void DummyLogger::submit() noexcept {};
void DummyLogger::submit(uint64_t timeout) noexcept {};

void DummyLogger::log_entry(LogEntry entry) noexcept {};
