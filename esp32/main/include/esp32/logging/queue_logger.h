#ifndef ESP_QUEUE_LOGGER_H
#define ESP_QUEUE_LOGGER_H
#include "freertos/idf_additions.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/logger.h"
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>

class ESPSerialLogger : public LoggerInterface {
private:
  QueueInterface<LogEntry> *msg_queue;
  uint64_t dropped_count = 0;
  char buf[MAX_LOG_SIZE];

public:
  ESPSerialLogger(QueueInterface<LogEntry> *msg_queue,
                  LogSerializerInterface *serializer);
  void log_entry(LogEntry log_entry) noexcept override;

  void submit(uint64_t timeout) noexcept override;
  void submit() noexcept override;
};

#endif
