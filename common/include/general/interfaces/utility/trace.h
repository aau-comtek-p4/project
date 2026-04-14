#ifndef TRACE_H
#define TRACE_H

#include "general/common.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/context.h"
#include "general/misc/shutdown.h"
#include <coroutine>
#include <cstddef>
#define TRACE_TAG "TRACE"
#define TRACE_ERROR_TAG "TRACE ERROR"

#include <cstdint>
class Trace {
public:
  uint64_t id = 0;
  uint64_t parent_id = 0;
  uint64_t start_time_ns = 0;
  uint64_t last_suspend_ns = 0;
  uint64_t duration_ns = 0;
  uint64_t actual_duration_ns = 0;
  bool started = false;
  uint64_t name_id;

  void set_name(uint64_t name_id);
  void start();
  void end();
  void suspend_trace();
  void resume_trace();
  void add_time(uint64_t time);
  void add_actual_time(uint64_t time);

  void print();
};

#endif
