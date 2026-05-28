#ifndef LOGGER_INTERFACE_H
#define LOGGER_INTERFACE_H

#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/errors.h"
#include "general/misc/names.h"
#include <atomic>
#include <cstdint>
#define LOG_WARNING_PREFIX "\033[33m"
#define LOG_DEBUG_PREFIX "\033[38;2;124;159;255m"
#define LOG_INFO_PREFIX "\033[37m"
#define LOG_ERROR_PREFIX "\033[38;2;255;21;60m"

#define LOGGER_TAG "LOGGER"
#define LOGGER_ERROR_TAG "LOGGER ERROR"

#define MAX_LOG_SIZE 256

#define ALLOCATOR_LOGGING 0
#define TRACE_LOGGING 0
#define COROUTINE_LOGGING 0
#define IO_LOGGING 0
#define DEADLINE_LOGGING 0

enum CoRoutineType : uint8_t {
  ROUTINE_JOB,
  ROUTINE_TASK,
};

enum IOType : uint8_t {
  READ,
  WRITE,
  OPEN,
  CLOSE,
  ACCEPT,
  RECV,
  SEND,
  CONNECT,
};
// File must be final before end
// esp uses file for max size as it has no file io
enum IOMethod : uint8_t {
  IO_WIFI_TCP = 0,
  IO_WIFI_UDP = 1,
  IO_ESP_NOW = 2,
  IO_SERIAL = 3,
  IO_GPIO = 4,
  IO_FILE = 5,
  IO_END = 6,
};
enum LogLevel : uint8_t {
  LOG_INFO,
  LOG_DEBUG,
  LOG_ERROR,
  LOG_WARNING,
};

enum LogReason : uint8_t {
  // Coroutine
  REASON_COROUTINE_STARTED,
  REASON_COROUTINE_FINISHED,
  REASON_COROUTINE_TIMEOUT,
  REASON_COROUTINE_SUSPENDED,
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
  REASON_BACKTRACE,
  //
  REASON_TRACE_PRINT,
  // Logs dropped
  REASON_DROPPED_LOGS,
  //
  LOG_METRIC,
  LOG_STAT_METRIC,

};
class Trace;
union LogPayload {
  struct {
    name_type_t name_index;
    name_type_t parent_name_index;
  } coroutine_started;
  struct {
    uint64_t actual_time_ns;
    uint64_t trace_index;
    name_type_t name_index;
    name_type_t parent_name_index;
  } coroutine_finished;
  struct {
    name_type_t name_index;
    name_type_t parent_name_index;
    uint64_t trace_index;
  } coroutine_timeout;
  struct {
    name_type_t name_index;
    uint64_t actual_time_ns;
  } coroutine_suspended;
  struct {
    name_type_t name_index;
    uint64_t amount_allocated;
    uint64_t amount_left;
  } queue_treshold;
  struct {
    name_type_t name_index;
    uint64_t amount_allocated;
    uint64_t amount_left;
  } allocator_allocation;
  struct {
    name_type_t name_index;
    uint64_t amount_allocated;
    uint64_t amount_left;
    uint64_t bucket_size;
  } bucket_allocator_allocation;
  struct {
    name_type_t name_index;
    uint64_t amount_allocated;
    uint64_t amount_left;
  } allocator_threshold;
  struct {
    name_type_t name_index;
    uint64_t amount_freed;
    uint64_t amount_left;
  } allocator_freed;
  struct {
    uint64_t trace_index;
    uint64_t parent_name_index;
    name_type_t name_index;
    IOMethod method;
    IOType type;
  } io_timeout;
  struct {
    name_type_t name_index;
    name_type_t parent_name_index;
    uint64_t trace_index;
    int32_t result;
    IOMethod method;
    IOType type;
  } io_completed;

  struct {
    name_type_t name_index;
    name_type_t parent_name_index;
    uint64_t trace_index;
    int32_t error_no;
    IOMethod method;
    IOType type;
  } io_error;
  struct {
    uint64_t absolute_deadline_ms;
  } new_deadline;

  struct {
    char debug[32] = {0};
  } debug;
  struct {
    ErrorWrapper error_wrapper;
  } shutdown;
  struct {
    uint64_t actual_duration_ns;
    uint64_t trace_id;
    uint64_t parent_id;
    name_type_t name_id;
  } trace_print;
  struct {
    uint64_t dropped_amount;
  } logs_dropped;
  struct {
    StatMetric stat_metric = {};
    StatMetricType metric_type;
  } log_stat_metric;
  struct {
    uint64_t count;
    MetricType metric_type;
  } log_metric;
  struct {
    char mem_addr[32] = {0};
  } log_backtrace;
  LogPayload() {};
  ~LogPayload() {};
};

struct LogEntry {
  LogPayload payload;
  uint64_t timestamp;
  uint32_t log_count;
  LogLevel severity;
  LogReason reason;
  uint8_t device_id;
  uint8_t device_type;
};

LogLevel get_severity(LogReason reason);

namespace logging {
LogEntry log_coroutine_start(uint64_t name_index, uint64_t parent_name_index);
LogEntry log_coroutine_finished(uint64_t name_index, uint64_t parent_name_index,
                                uint64_t actual_time_elapsed_ns,
                                uint64_t trace_index);
LogEntry log_coroutine_timeout(uint64_t name_index, uint64_t parent_name_index,
                               uint64_t trace_index);

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

LogEntry log_io_timeout(IOMethod io_method, IOType io_type, uint64_t name_index,
                        uint64_t parent_name_index, uint64_t trace_index);

LogEntry log_io_complete(IOMethod io_method, IOType io_type,
                         uint64_t name_index, uint64_t parent_name_index,
                         uint64_t trace_index, uint64_t result);

LogEntry log_io_error(IOMethod io_method, IOType io_type, uint64_t name_index,
                      uint64_t parent_name_index, uint64_t trace_index,
                      uint64_t err_no);

LogEntry log_new_deadline(uint64_t absolute_deadline_ms);

__attribute__((format(printf, 1, 2))) LogEntry log_debug(const char *fmt, ...);

LogEntry log_backtrace(char *mem_addr);

LogEntry log_shutdown(ErrorWrapper error_wrapper);

LogEntry log_trace_print(uint64_t name_id, uint64_t trace_id,
                         uint64_t parent_id, uint64_t actual_duration_ns);
LogEntry log_dropped_logs(uint64_t dropped_amount);
LogEntry log_stat_metric(StatMetricType metric_type, StatMetric stat_metric);
LogEntry log_metric(MetricType metric_type, uint64_t count);
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
  uint32_t log_count = 0;
  QueueInterface<LogEntry> *msg_queue;
  char out_buf[MAX_LOG_SIZE];
  LogSerializerInterface *serializer;

  virtual void log_entry(LogEntry log_entry) noexcept = 0;

  virtual void submit(uint64_t timeout) noexcept = 0;
  virtual void submit() noexcept = 0;
};
const char *parse_log_level(LogLevel log_level);

const char *parse_reason(LogReason reason);

#endif
