#ifndef TRACE_H
#define TRACE_H

#include "general/interfaces/storage/allocator.h"
#include <coroutine>
#define TRACE_TAG "TRACE"
#define TRACE_ERROR_TAG "TRACE ERROR"
#define MAX_TRACE_NAME_LENGTH 20

#include <cstdint>
struct Trace {
  uint64_t start_time_ns;
  uint64_t last_suspend_ns;
  uint64_t duration_ns;
  uint64_t actual_duration_ns;
  Trace *first_child = nullptr;
  Trace *last_child = nullptr;
  Trace *next_sibling = nullptr;
  bool started = false;
  ;

  char name[MAX_TRACE_NAME_LENGTH] = {0};

  void append_child(Trace *trace);
  void set_name(const char *);
  void start();
  void suspend_trace();
  void resume_trace();
  void add_time(uint64_t time);
  void add_actual_time(uint64_t time);

  void print();
};

struct TraceHandler {
private:
  AllocatorInterface *trace_allocator;
  uint64_t max_traces;

public:
  TraceHandler(AllocatorInterface *trace_allocator, uint64_t max_traces);
  Trace *get_trace();
  void clear_trace(Trace *trace);
};

#endif
