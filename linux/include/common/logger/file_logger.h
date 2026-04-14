#ifndef FILE_LOGGER_H
#define FILE_LOGGER_H

#include "common/io/io.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/logger.h"
#include <cstdint>
#include <cstdio>
#define MAX_LOGS 200
#define LOG_FILE_NAME "log.txt"
#define EVENT_LOG_FILE_NAME "event_logs.txt"
class FileLogger : public LoggerInterface {
private:
  Queue<LogEntry, MAX_LOGS> msg_queue;
  int file_descriptor;
  char out_buf[MAX_LOG_SIZE];
  LogSerializerInterface *serializer;

public:
  FileLogger(LogSerializerInterface *serializer);
  void log_entry(LogEntry entry) noexcept override;

  void submit(uint64_t timeout) noexcept override;
  void submit() noexcept override;
};

#endif
