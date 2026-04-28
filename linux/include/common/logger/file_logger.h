#ifndef FILE_LOGGER_H
#define FILE_LOGGER_H

#include "common/io/io.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/logger.h"
#include <cstdint>
#include <cstdio>
#define MAX_LOG_FILE_NAME_SIZE 30
#define LOG_FILE_NAME "log-" CONFIG_PROJECT_ID ".txt"
class FileLogger : public LoggerInterface {
private:
  QueueInterface<LogEntry> *msg_queue;
  int file_descriptor;
  char out_buf[MAX_LOG_SIZE];
  LogSerializerInterface *serializer;
  uint64_t dropped_count = 0;

public:
  FileLogger(QueueInterface<LogEntry> *msg_queue,
             LogSerializerInterface *serializer);
  void log_entry(LogEntry entry) noexcept override;

  void submit(uint64_t timeout) noexcept override;
  void submit() noexcept override;
};

#endif
