#ifndef TRACE_H
#define TRACE_H

#define TRACE_TAG "TRACE"
#define TRACE_ERROR_TAG "TRACE ERROR"

#include <cstdint>
class Trace {
public:
  uint64_t id = 0;
  uint64_t parent_id = 0;
  uint64_t start_time_ns = 0;
  uint64_t last_suspend_ns = 0;
  uint64_t actual_duration_ns = 0;
  bool started = false;
  uint64_t name_id;

  void set_name(uint64_t name_id);
  void start();
  void end();
  void suspend_trace();
  void suspend_trace(bool print);
  void resume_trace();
  void add_actual_time(uint64_t time);

  void print();
};

#endif
