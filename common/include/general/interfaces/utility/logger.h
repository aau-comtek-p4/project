#ifndef LOGGER_INTERFACE_H
#define LOGGER_INTERFACE_H

#include "general/misc/errors.h"
#include <cstdint>
#define LOG_WARNING_PREFIX "\033[33m"
#define LOG_DEBUG_PREFIX "\033[38;2;124;159;255m"
#define LOG_INFO_PREFIX "\033[37m"
#define LOG_ERROR_PREFIX "\033[38;2;255;21;60m"

#define LOGGER_TAG "LOGGER"
#define LOGGER_ERROR_TAG "LOGGER ERROR"

#define MAX_LOG_SIZE 256
enum CoRoutineType {
  ROUTINE_JOB,
  ROUTINE_TASK,
};

enum IOType {
  READ,
  WRITE,
  OPEN,
  CLOSE,
  ACCEPT,
  RECV,
  SEND,
  CONNECT,
};
enum IOMethod {
  IO_WIFI_TCP = 0,
  IO_WIFI_UDP = 1,
  IO_ESP_NOW = 2,
  IO_SERIAL = 3,
  IO_GPIO = 4,
  IO_FILE = 5,
  IO_END = 6,
};
enum LogLevel {
  LOG_INFO,
  LOG_DEBUG,
  LOG_ERROR,
  LOG_WARNING,
};

enum LogReason {
  // Coroutine
  REASON_COROUTINE_STARTED,
  REASON_COROUTINE_FINISHED,
  REASON_COROUTINE_TIMEOUT,
  REASON_COROUTINE_SUSPENDED,
  // Clock
  REASON_MISSED_TICK_OCCURED,
  REASON_TICK_COMPLETE,
  //
  REASON_QUEUE_THRESHOLD,
  // Allocator
  REASON_ALLOCATOR_ALLOCATION,
  REASON_BUCKET_ALLOCATOR_ALLOCATION,
  REASON_ALLOCATOR_TRHESHOLD,
  REASON_ALLOCATOR_FREE,
  // IO
  REASON_IO_TIMEOUT,
  REASON_IO_COMPLETED,
  REASON_IO_ERROR,
  // DEADLINE
  REASON_NEW_DEADLINE,
  //
  REASON_DEBUG,
  //
  REASON_SHUTDOWN,
  //
  REASON_TRACE_PRINT,
};
class Trace;
union LogPayload {
  struct {
    uint64_t name_index;
    uint64_t parent_name_index;
  } coroutine_started;
  struct {
    uint64_t name_index;
    uint64_t parent_name_index;
    uint64_t actual_time_ns;
    uint64_t trace_index;
  } coroutine_finished;
  struct {
    uint64_t name_index;
    uint64_t parent_name_index;
    uint64_t actual;
    uint64_t deadline;
    uint64_t delta;
    uint64_t trace_index;
  } coroutine_timeout;

  struct {
    uint64_t name_index;
    uint64_t actual_time_ns;
  } coroutine_suspended;
  struct {
    uint64_t deadline;
    uint64_t actual;
    uint64_t delta;
  } missed_tick_occured;
  struct {
    uint64_t deadline;
    uint64_t actual;
    uint64_t time_remaining;
  } tick_complete;
  struct {
    uint64_t name_index;
    uint64_t amount_allocated;
    uint64_t amount_left;
  } queue_treshold;
  struct {
    uint64_t name_index;
    uint64_t amount_allocated;
    uint64_t amount_left;
  } allocator_allocation;
  struct {
    uint64_t name_index;
    uint64_t amount_allocated;
    uint64_t amount_left;
    uint64_t bucket_size;
  } bucket_allocator_allocation;
  struct {
    uint64_t name_index;
    uint64_t amount_allocated;
    uint64_t amount_left;
  } allocator_threshold;
  struct {
    uint64_t name_index;
    uint64_t amount_freed;
    uint64_t amount_left;
  } allocator_freed;

  struct {
    IOMethod method;
    IOType type;
    uint64_t parent_name_index;
    uint64_t deadline;
    uint64_t actual;
    uint64_t delta;
  } io_timeout;

  struct {
    IOMethod method;
    IOType type;
    uint64_t parent_name_index;
    int result;
  } io_completed;

  struct {
    IOMethod method;
    IOType type;
    uint64_t parent_name_index;
    int error_no;
  } io_error;
  struct {
    uint64_t deadline_tick;
  } new_deadline;

  struct {
    char debug[48];
  } debug;
  struct {
    ErrorWrapper error_wrapper;
  } shutdown;
  struct {
    uint64_t duration_ns;
    uint64_t actual_duration_ns;
    uint64_t name_id;
    uint64_t trace_id;
    uint64_t parent_id;
  } trace_print;
};

struct LogEntry {
  LogReason reason;
  LogLevel severity;
  uint64_t timestamp;
  uint64_t tick_count;
  LogPayload payload;
};
LogLevel get_severity(LogReason reason);

namespace logging {
LogEntry log_coroutine_start(uint64_t name_index, uint64_t parent_name_index);
LogEntry log_coroutine_finished(uint64_t name_index, uint64_t parent_name_index,
                                uint64_t actual_time_elapsed_ns,
                                uint64_t trace_index);
LogEntry log_coroutine_timeout(uint64_t name_index, uint64_t parent_name_index,
                               uint64_t trace_index, uint64_t deadline,
                               uint64_t actual);

LogEntry log_coroutine_suspended(uint64_t name_index,
                                 uint64_t actual_time_elapsed);
LogEntry log_clock_miss(uint64_t deadline, uint64_t actual_time_ns);
LogEntry log_clock_tick(uint64_t deadline, uint64_t actual_time_ns);
LogEntry log_bucket_allocator_allocation(uint64_t name_index,
                                         uint64_t amount_allocated,
                                         uint64_t amount_left,
                                         uint64_t bucket_size);
LogEntry log_queue_threshold(uint64_t name_index, uint64_t amount_allocated,
                             uint64_t amount_left);

LogEntry log_allocator_allocation(uint64_t name_index,
                                  uint64_t amount_allocated,
                                  uint64_t amount_left);
LogEntry log_allocator_threshold_reached(uint64_t name_index,
                                         uint64_t amount_allocated,
                                         uint64_t amount_left);
LogEntry log_allocator_free(uint64_t name_index, uint64_t amount_allocated,
                            uint64_t amount_left);

LogEntry log_io_timeout(IOMethod io_method, IOType io_type,
                        uint64_t parent_name_index, uint64_t deadline,
                        uint64_t actual);

LogEntry log_io_complete(IOMethod io_method, IOType io_type,
                         uint64_t parent_name_index, uint64_t result);

LogEntry log_io_error(IOMethod io_method, IOType io_type,
                      uint64_t parent_name_index, uint64_t errno);

LogEntry log_new_deadline(uint64_t deadline_tick);

LogEntry log_debug(const char *debug_str);

LogEntry log_shutdown(ErrorWrapper error_wrapper);

LogEntry log_trace_print(uint64_t name_id, uint64_t trace_id,
                         uint64_t parent_id, uint64_t duration_ns,
                         uint64_t actual_duration_ns);
}; // namespace logging

const char *parse_io_type(IOType type);
const char *parse_io_method(IOMethod method);
class LogSerializerInterface {
public:
  virtual uint64_t serialize(char *buf, uint64_t max_write, LogEntry entry) = 0;
};

class JsonLogSerializer : public LogSerializerInterface {
public:
  uint64_t serialize(char *buf, uint64_t max_write, LogEntry entry) override;
};

class LoggerInterface {
public:
  virtual void log_entry(LogEntry log_entry) noexcept = 0;

  virtual void submit(uint64_t timeout) noexcept = 0;
  virtual void submit() noexcept = 0;
};
const char *parse_log_level(LogLevel log_level);

const char *parse_reason(LogReason reason);

#endif
