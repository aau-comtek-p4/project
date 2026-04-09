#ifndef TRACE_H
#define TRACE_H

#include "general/interfaces/storage/allocator.h"

#define TRACE_TAG "TRACE"
#define TRACE_ERROR_TAG "TRACE ERROR"

#include <cstdint>
struct Trace {
  char name[20] = {0};
  uint64_t start_time;
  uint64_t duration;
  Trace *first_child;
  Trace *next_sibling;
};

struct TraceHandler {
private:
  AllocatorInterface *trace_allocator;

public:
  TraceHandler(AllocatorInterface *trace_allocator);
  Trace *get_trace();
};

#endif
