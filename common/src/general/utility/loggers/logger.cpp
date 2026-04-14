#include "general/interfaces/utility/logger.h"
#include "general/common.h"
#include "general/interfaces/utility/clock.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>
const char *parse_io_type(IOType type) {
  switch (type) {
  case IOType::OPEN:
    return "open";
  case IOType::READ:
    return "read";
  case IOType::WRITE:
    return "write";
  case IOType::CLOSE:
    return "close";
  case IOType::RECV:
    return "recv";
  case IOType::ACCEPT:
    return "accept";
  case IOType::SEND:
    return "send";
  case IOType::CONNECT:
    return "connect";
  }
  return "unknown";
}

const char *parse_io_method(IOMethod io_method) {
  switch (io_method) {
  case IOMethod::IO_WIFI_TCP:
    return "io_wifi_tcp";
  case IOMethod::IO_WIFI_UDP:
    return "io_wifi_udp";
  case IOMethod::IO_ESP_NOW:
    return "io_esp_now";
  case IOMethod::IO_SERIAL:
    return "io_serial";
  case IOMethod::IO_GPIO:
    return "io_gpio";
  case IOMethod::IO_FILE:
    return "io_file";
  case IOMethod::IO_END:
    return "io_end";
  }
  return "unknown";
}
const char *parse_coroutine_type(CoRoutineType coroutine_type) {
  switch (coroutine_type) {
  case CoRoutineType::ROUTINE_JOB:
    return "job";
  case CoRoutineType::ROUTINE_TASK:
    return "task";
  }
  return "unknown";
}

LogLevel get_severity(LogReason reason) {
  switch (reason) {
    // INFO
  case LogReason::REASON_COROUTINE_STARTED:
  case LogReason::REASON_COROUTINE_FINISHED:
  case LogReason::REASON_COROUTINE_SUSPENDED:
  case LogReason::REASON_TICK_COMPLETE:
  case LogReason::REASON_ALLOCATOR_ALLOCATION:
  case LogReason::REASON_ALLOCATOR_FREE:
  case LogReason::REASON_IO_COMPLETED:
  case LogReason::REASON_BUCKET_ALLOCATOR_ALLOCATION:
  case LogReason::REASON_NEW_DEADLINE:
  case LogReason::REASON_TRACE_PRINT:
    return LogLevel::LOG_INFO;

    // WARNING
  case LogReason::REASON_COROUTINE_TIMEOUT:
  case LogReason::REASON_MISSED_TICK_OCCURED:
  case LogReason::REASON_ALLOCATOR_TRHESHOLD:
  case LogReason::REASON_IO_TIMEOUT:
  case LogReason::REASON_QUEUE_THRESHOLD:
    return LogLevel::LOG_WARNING;
    // ERROR

  case LogReason::REASON_SHUTDOWN:
  case LogReason::REASON_IO_ERROR:
    return LogLevel::LOG_ERROR;
  case LogReason::REASON_DEBUG:
    return LogLevel::LOG_DEBUG;
  }
  // DEBUG
  return LogLevel::LOG_DEBUG;
}

LogEntry logging::log_coroutine_start(uint64_t name_index,
                                      uint64_t parent_name_index) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_COROUTINE_STARTED;
  entry.severity = get_severity(entry.reason);
  entry.payload.coroutine_started.name_index = name_index;
  entry.payload.coroutine_started.parent_name_index = parent_name_index;
  return entry;
}
LogEntry logging::log_coroutine_finished(uint64_t name_index,
                                         uint64_t parent_name_index,
                                         uint64_t actual_time_elapsed_ns,
                                         uint64_t trace_index) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_COROUTINE_FINISHED;
  entry.severity = get_severity(entry.reason);
  entry.payload.coroutine_finished.name_index = name_index;
  entry.payload.coroutine_finished.actual_time_ns = actual_time_elapsed_ns;
  entry.payload.coroutine_finished.trace_index = trace_index;
  entry.payload.coroutine_finished.parent_name_index = parent_name_index;
  return entry;
}
LogEntry logging::log_coroutine_timeout(uint64_t name_index,
                                        uint64_t parent_name_index,
                                        uint64_t trace_index, uint64_t deadline,
                                        uint64_t actual) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_COROUTINE_TIMEOUT;
  entry.severity = get_severity(entry.reason);

  entry.payload.coroutine_timeout.name_index = name_index;
  entry.payload.coroutine_timeout.parent_name_index = parent_name_index;
  entry.payload.coroutine_timeout.trace_index = trace_index;
  entry.payload.coroutine_timeout.deadline = deadline;
  entry.payload.coroutine_timeout.actual = actual;
  entry.payload.coroutine_timeout.delta = actual - deadline;
  return entry;
}
LogEntry logging::log_coroutine_suspended(uint64_t name_index,
                                          uint64_t actual_time_elapsed) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_COROUTINE_SUSPENDED;
  entry.severity = get_severity(entry.reason);

  entry.payload.coroutine_suspended.name_index = name_index;
  entry.payload.coroutine_suspended.actual_time_ns = actual_time_elapsed;
  return entry;
}

LogEntry logging::log_clock_miss(uint64_t deadline, uint64_t actual_time_ns) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_MISSED_TICK_OCCURED;
  entry.severity = get_severity(entry.reason);

  entry.payload.missed_tick_occured.actual = actual_time_ns;
  entry.payload.missed_tick_occured.deadline = deadline;
  entry.payload.missed_tick_occured.delta = actual_time_ns - deadline;
  return entry;
}
LogEntry logging::log_clock_tick(uint64_t deadline, uint64_t actual_time_ns) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_TICK_COMPLETE;
  entry.severity = get_severity(entry.reason);

  entry.payload.tick_complete.deadline = deadline;
  entry.payload.tick_complete.actual = actual_time_ns;
  entry.payload.tick_complete.time_remaining = deadline - actual_time_ns;
  return entry;
}
LogEntry logging::log_queue_threshold(uint64_t name_index,
                                      uint64_t amount_allocated,
                                      uint64_t amount_left) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_QUEUE_THRESHOLD;
  entry.severity = get_severity(entry.reason);

  entry.payload.queue_treshold.amount_allocated = amount_allocated;
  entry.payload.queue_treshold.amount_left = amount_left;
  entry.payload.queue_treshold.name_index = name_index;
  return entry;
}

LogEntry logging::log_bucket_allocator_allocation(uint64_t name_index,
                                                  uint64_t amount_allocated,
                                                  uint64_t amount_left,
                                                  uint64_t bucket_size) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_BUCKET_ALLOCATOR_ALLOCATION;
  entry.severity = get_severity(entry.reason);

  entry.payload.bucket_allocator_allocation.amount_allocated = amount_allocated;
  entry.payload.bucket_allocator_allocation.amount_left = amount_left;
  entry.payload.bucket_allocator_allocation.name_index = name_index;
  entry.payload.bucket_allocator_allocation.bucket_size = bucket_size;

  return entry;
}
LogEntry logging::log_allocator_allocation(uint64_t name_index,
                                           uint64_t amount_allocated,
                                           uint64_t amount_left) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_ALLOCATOR_ALLOCATION;
  entry.severity = get_severity(entry.reason);

  entry.payload.allocator_allocation.amount_allocated = amount_allocated;
  entry.payload.allocator_allocation.amount_left = amount_left;
  entry.payload.allocator_allocation.name_index = name_index;
  return entry;
}
LogEntry logging::log_allocator_threshold_reached(uint64_t amount_allocated,
                                                  uint64_t amount_left,
                                                  uint64_t name_index) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_ALLOCATOR_TRHESHOLD;
  entry.severity = get_severity(entry.reason);

  entry.payload.allocator_threshold.amount_left = amount_left;
  entry.payload.allocator_threshold.amount_allocated = amount_allocated;
  entry.payload.allocator_threshold.name_index = name_index;
  return entry;
}
LogEntry logging::log_allocator_free(uint64_t name_index, uint64_t amount_freed,
                                     uint64_t amount_left) {

  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_ALLOCATOR_FREE;
  entry.severity = get_severity(entry.reason);

  entry.payload.allocator_freed.amount_left = amount_left;
  entry.payload.allocator_freed.amount_freed = amount_freed;
  entry.payload.allocator_freed.name_index = name_index;
  return entry;
}

LogEntry logging::log_io_timeout(IOMethod io_method, IOType io_type,
                                 uint64_t parent_name_index, uint64_t deadline,
                                 uint64_t actual) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_IO_TIMEOUT;
  entry.severity = get_severity(entry.reason);

  entry.payload.io_timeout.method = io_method;
  entry.payload.io_timeout.type = io_type;
  entry.payload.io_timeout.parent_name_index = parent_name_index;
  entry.payload.io_timeout.deadline = deadline;
  entry.payload.io_timeout.actual = actual;
  entry.payload.io_timeout.delta = actual - deadline;
  return entry;
}

LogEntry logging::log_io_complete(IOMethod io_method, IOType io_type,
                                  uint64_t parent_name_index,
                                  uint64_t io_result) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_IO_COMPLETED;
  entry.severity = get_severity(entry.reason);

  entry.payload.io_completed.parent_name_index = parent_name_index;
  entry.payload.io_completed.method = io_method;
  entry.payload.io_completed.type = io_type;
  entry.payload.io_completed.result = io_result;
  return entry;
}

LogEntry logging::log_io_error(IOMethod io_method, IOType io_type,
                               uint64_t parent_name_index, uint64_t err_no) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_IO_COMPLETED;
  entry.severity = get_severity(entry.reason);

  entry.payload.io_error.parent_name_index = parent_name_index;
  entry.payload.io_error.method = io_method;
  entry.payload.io_error.type = io_type;
  entry.payload.io_error.error_no = err_no;
  return entry;
}

LogEntry logging::log_new_deadline(uint64_t deadline_tick) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_NEW_DEADLINE;
  entry.severity = get_severity(entry.reason);

  entry.payload.new_deadline.deadline_tick = deadline_tick;
  return entry;
}
LogEntry logging::log_debug(const char *debug_str) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_DEBUG;
  entry.severity = get_severity(entry.reason);

  uint64_t str_len = strlen(debug_str);

  strncpy(entry.payload.debug.debug, debug_str, str_len);
  entry.payload.debug.debug[str_len] = 0;
  return entry;
}

LogEntry logging::log_shutdown(ErrorWrapper error_wrapper) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_SHUTDOWN;
  entry.severity = get_severity(entry.reason);

  entry.payload.shutdown.error_wrapper = error_wrapper;
  return entry;
}

LogEntry logging::log_trace_print(uint64_t name_id, uint64_t trace_id,
                                  uint64_t parent_id, uint64_t duration_ns,
                                  uint64_t actual_duration_ns) {
  LogEntry entry;
  entry.timestamp = program_ctxt->clock->rt_since_start_ns();
  entry.tick_count = program_ctxt->clock->tick_now();
  entry.reason = LogReason::REASON_TRACE_PRINT;
  entry.severity = get_severity(entry.reason);

  entry.payload.trace_print.name_id = name_id;
  entry.payload.trace_print.trace_id = trace_id;
  entry.payload.trace_print.parent_id = parent_id;
  entry.payload.trace_print.duration_ns = duration_ns;
  entry.payload.trace_print.actual_duration_ns = actual_duration_ns;
  return entry;
}
const char *parse_log_level(LogLevel log_level) {
  switch (log_level) {
  case LOG_ERROR:
    return "error";

  case LOG_WARNING:
    return "warning";

  case LOG_INFO:
    return "info";

  case LOG_DEBUG:
    return "debug";
  }
  return "unknown";
}

const char *parse_reason(LogReason reason) {
  switch (reason) {

  case LogReason::REASON_COROUTINE_STARTED:
    return "coroutine_started";
  case LogReason::REASON_COROUTINE_FINISHED:
    return "coroutine_finished";
  case LogReason::REASON_COROUTINE_SUSPENDED:
    return "coroutine_suspended";
  case LogReason::REASON_TICK_COMPLETE:
    return "tick_complete";
  case LogReason::REASON_ALLOCATOR_ALLOCATION:
    return "allocator_allocation";
  case LogReason::REASON_BUCKET_ALLOCATOR_ALLOCATION:
    return "bucket_allocator_allocation";
  case LogReason::REASON_QUEUE_THRESHOLD:
    return "queue_threshold";
  case LogReason::REASON_ALLOCATOR_FREE:
    return "allocator_free";
  case LogReason::REASON_IO_COMPLETED:
    return "io_completed";
  case LogReason::REASON_COROUTINE_TIMEOUT:
    return "coroutine_timeout";
  case LogReason::REASON_MISSED_TICK_OCCURED:
    return "missed_tick";
  case LogReason::REASON_ALLOCATOR_TRHESHOLD:
    return "allocator_threshold";
  case LogReason::REASON_IO_TIMEOUT:
    return "io_timeout";
  case LogReason::REASON_IO_ERROR:
    return "io_error";
  case LogReason::REASON_NEW_DEADLINE:
    return "new_deadline";
  case LogReason::REASON_DEBUG:
    return "debug";
  case LogReason::REASON_SHUTDOWN:
    return "shutdown";
  case LogReason::REASON_TRACE_PRINT:
    return "trace_print";
  }
  return "io_unkown";
}
uint64_t serializer_helper(char *buf, uint64_t max_entry, LogEntry log_entry) {
  return snprintf(buf, max_entry,
                  "{\"ts\":%" PRIu64 ",\"tick\":%" PRIu64
                  ",\"serverity\":\"%s\",\"reason\":\"%s\",\"payload\":{",
                  log_entry.timestamp, log_entry.tick_count,
                  parse_log_level(log_entry.severity),
                  parse_reason(log_entry.reason));
}
uint64_t JsonLogSerializer::serialize(char *buf, uint64_t max_entry,
                                      LogEntry log_entry) {

  uint64_t header_size = serializer_helper(buf, max_entry, log_entry);
  switch (log_entry.reason) {
  case LogReason::REASON_IO_COMPLETED:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"result\":%" PRIi32
                    ",\"type\":\"%s\",\"method\":\"%s\",\"caller\":\"%s\"}}\n",
                    log_entry.payload.io_completed.result,
                    parse_io_type(log_entry.payload.io_completed.type),
                    parse_io_method(log_entry.payload.io_completed.method),
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.io_completed.parent_name_index)) +
           header_size;
  case LogReason::REASON_IO_TIMEOUT:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"type\":\"%s\",\"method\":\"%s\",\"caller\":\"%s\","
                    "\"actual\":%" PRIu64 ",\"deadline\":%" PRIu64
                    ",\"delta\":%" PRIu64 "}}\n",
                    parse_io_type(log_entry.payload.io_timeout.type),
                    parse_io_method(log_entry.payload.io_timeout.method),
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.io_timeout.parent_name_index),
                    log_entry.payload.io_timeout.actual,
                    log_entry.payload.io_timeout.deadline,
                    log_entry.payload.io_timeout.delta) +
           header_size;
  case LogReason::REASON_IO_ERROR:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"type\":\"%s\",\"method\":\"%s\",\"caller\":\"%s\","
                    "\"errno\":%i,\"strerror\":\"%s\"}}\n",
                    parse_io_type(log_entry.payload.io_error.type),
                    parse_io_method(log_entry.payload.io_error.method),
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.io_error.parent_name_index),
                    log_entry.payload.io_error.error_no,
                    strerror(log_entry.payload.io_error.error_no)) +
           header_size;
  case LogReason::REASON_COROUTINE_STARTED:
    return snprintf(
               buf + header_size, max_entry - header_size,
               "\"name\":\"%s\",\"parent\":\"%s\"}}\n",
               program_ctxt->name_lookup->get_name(
                   log_entry.payload.coroutine_started.name_index),
               program_ctxt->name_lookup->get_name(
                   log_entry.payload.coroutine_started.parent_name_index)) +
           header_size;
  case LogReason::REASON_COROUTINE_SUSPENDED:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"name\":\"%s\","
                    "\"actual_ns\":%" PRIu64 "}}\n",
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.coroutine_suspended.name_index),
                    log_entry.payload.coroutine_suspended.actual_time_ns) +
           header_size;
  case LogReason::REASON_MISSED_TICK_OCCURED:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"deadline\":%" PRIu64 ",\"actual\":%" PRIu64
                    ",\"delta\":%" PRIu64 "}}\n",
                    log_entry.payload.missed_tick_occured.deadline,
                    log_entry.payload.missed_tick_occured.actual,
                    log_entry.payload.missed_tick_occured.delta) +
           header_size;
  case LogReason::REASON_TICK_COMPLETE:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"deadline\":%" PRIu64 ",\"actual\":%" PRIu64
                    ",\"remaining\":%" PRIu64 "}}\n",
                    log_entry.payload.tick_complete.deadline,
                    log_entry.payload.tick_complete.actual,
                    log_entry.payload.tick_complete.time_remaining) +
           header_size;
  case LogReason::REASON_ALLOCATOR_ALLOCATION:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"name\":\"%s\",\"allocated\":%" PRIu64
                    ",\"remaining\":%" PRIu64 "}}\n",
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.allocator_allocation.name_index),
                    log_entry.payload.allocator_allocation.amount_allocated,
                    log_entry.payload.allocator_allocation.amount_left) +
           header_size;
  case LogReason::REASON_BUCKET_ALLOCATOR_ALLOCATION:
    return snprintf(
               buf + header_size, max_entry - header_size,
               "\"name\":\"%s\",\"allocated\":%" PRIu64
               ",\"remaining\":%" PRIu64 ",\"bucket_size\":%" PRIu64 "}}\n",
               program_ctxt->name_lookup->get_name(
                   log_entry.payload.bucket_allocator_allocation.name_index),
               log_entry.payload.bucket_allocator_allocation.amount_allocated,
               log_entry.payload.bucket_allocator_allocation.amount_left,
               log_entry.payload.bucket_allocator_allocation.bucket_size) +
           header_size;
  case LogReason::REASON_ALLOCATOR_TRHESHOLD:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"name\":\"%s\",\"allocated\":%" PRIu64
                    ",\"remaining\":%" PRIu64 "}}\n",
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.allocator_threshold.name_index),
                    log_entry.payload.allocator_threshold.amount_allocated,
                    log_entry.payload.allocator_threshold.amount_left) +
           header_size;
  case LogReason::REASON_QUEUE_THRESHOLD:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"name\":\"%s\",\"allocated\":%" PRIu64
                    ",\"remaining\":%" PRIu64 "}}\n",
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.queue_treshold.name_index),
                    log_entry.payload.queue_treshold.amount_allocated,
                    log_entry.payload.queue_treshold.amount_left) +
           header_size;
  case LogReason::REASON_ALLOCATOR_FREE:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"name\":\"%s\",\"freed\":%" PRIu64
                    ",\"remaining\":%" PRIu64 "}}\n",
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.allocator_freed.name_index),
                    log_entry.payload.allocator_freed.amount_freed,
                    log_entry.payload.allocator_freed.amount_left) +
           header_size;
  case LogReason::REASON_COROUTINE_TIMEOUT:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"name\":\"%s\",\"parent\":\"%s\","
                    "\"trace_index\":%" PRIu64 ",\"deadline\":%" PRIu64
                    ",\"actual\":%" PRIu64 ",\"delta\":%" PRIu64 "}}\n",
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.coroutine_timeout.name_index),
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.coroutine_timeout.parent_name_index),
                    log_entry.payload.coroutine_timeout.trace_index,
                    log_entry.payload.coroutine_timeout.deadline,
                    log_entry.payload.coroutine_timeout.actual,
                    log_entry.payload.coroutine_timeout.delta) +
           header_size;
  case LogReason::REASON_COROUTINE_FINISHED:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"name\":\"%s\",\"parent\":\"%s\","
                    "\"trace_index\":%" PRIu64 ",\"actual\":%" PRIu64 "}}\n",
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.coroutine_finished.name_index),
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.coroutine_finished.parent_name_index),
                    log_entry.payload.coroutine_finished.trace_index,
                    log_entry.payload.coroutine_finished.actual_time_ns) +
           header_size;

  case LogReason::REASON_NEW_DEADLINE:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"tick\":%" PRIu64 "}}\n",
                    log_entry.payload.new_deadline.deadline_tick) +
           header_size;
  case LogReason::REASON_DEBUG:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"msg\":\"%s\"}}\n", log_entry.payload.debug.debug) +
           header_size;

  case LogReason::REASON_SHUTDOWN:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"msg\":\"%s\"}}\n",
                    custom_strerror(log_entry.payload.shutdown.error_wrapper)) +
           header_size;

  case LogReason::REASON_TRACE_PRINT:
    return snprintf(buf + header_size, max_entry - header_size,
                    "\"name\":\"%s\",\"id\":%" PRIu64 ",\"parent_id\":%" PRIu64
                    ",\"duration_ns\":%" PRIu64 ",\"actual_ns\":%" PRIu64
                    "}}\n",
                    program_ctxt->name_lookup->get_name(
                        log_entry.payload.trace_print.name_id),
                    log_entry.payload.trace_print.trace_id,
                    log_entry.payload.trace_print.parent_id,
                    log_entry.payload.trace_print.duration_ns,
                    log_entry.payload.trace_print.actual_duration_ns) +
           header_size;
  }
  return 0;
}
