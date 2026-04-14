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
  uint64_t id;
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

class TraceHandlerInterface {
public:
  virtual Trace *get_trace() = 0;
  virtual void clear_trace(Trace *trace) = 0;
};

template <uint64_t max_traces>
struct TraceHandler : public TraceHandlerInterface {
private:
  AllocatorInterface *trace_allocator;

public:
  TraceHandler(AllocatorInterface *trace_allocator);
  Trace *get_trace() override;
  void clear_trace(Trace *trace) override;
};

template <uint64_t max_traces>
TraceHandler<max_traces>::TraceHandler(AllocatorInterface *trace_allocator)
    : trace_allocator(trace_allocator) {}

template <uint64_t max_traces> Trace *TraceHandler<max_traces>::get_trace() {
  auto res = this->trace_allocator->allocate(sizeof(Trace));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
  auto trace_ptr = (Trace *)res.value();
  new (trace_ptr) Trace();
  program_ctxt->metrics->document_metric(MetricType::TRACE_CREATED);
  trace_ptr->id = program_ctxt->metrics->get_metric(MetricType::TRACE_CREATED);
  return trace_ptr;
}

template <uint64_t max_traces>
void TraceHandler<max_traces>::clear_trace(Trace *root) {
  auto res = this->trace_allocator->free(root);
  program_ctxt->metrics->document_metric(MetricType::TRACE_FREED);
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}

#endif
