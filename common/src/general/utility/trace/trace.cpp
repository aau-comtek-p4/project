#include "general/interfaces/utility/trace.h"
#include "general/common.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>
void Trace::append_child(Trace *trace) {
  if (!this->first_child) {
    this->first_child = trace;
    this->last_child = trace;
    return;
  }
  this->last_child->next_sibling = trace;
  this->last_child = trace;
  return;
}
void Trace::add_time(uint64_t time_ns) { this->duration_ns += time_ns; }
void Trace::add_actual_time(uint64_t time_ns) {
  this->actual_duration_ns += time_ns;
}
void Trace::set_name(const char *trace_name) {
  strncpy(this->name, trace_name, MAX_TRACE_NAME_LENGTH);
}
void Trace::start() {
  this->started = true;
  this->start_time_ns = program_ctxt->clock->rt_since_start_ns();
  this->last_suspend_ns = this->start_time_ns;
}
void Trace::suspend_trace() {
  if (this->started) {
    uint64_t additional_time =
        program_ctxt->clock->rt_since_start_ns() - this->last_suspend_ns;
    this->duration_ns += additional_time;
    this->actual_duration_ns += additional_time;
  }
}
void Trace::resume_trace() {
  if (this->started) {
    this->last_suspend_ns = program_ctxt->clock->rt_since_start_ns();
  }
}

struct TraceKeeper {
  Trace *trace;
  uint64_t generation;
};
void left_pad(char *buf, char symbol, uint64_t amount, uint64_t start) {
  for (uint64_t i = 0; i < amount; i++) {
    buf[start + i] = symbol;
  }
}
void Trace::print() {
  TraceKeeper stack[150];
  int top = 0;
  char buf[128] = {0};
  uint64_t total_written = 0;
  stack[top++] = TraceKeeper{.trace = this, .generation = 0};
  uint64_t gap = 2;

  while (top > 0) {
    TraceKeeper node = stack[--top];

    // Push all siblings onto stack
    if (node.trace->next_sibling) {
      stack[top++] = TraceKeeper{.trace = node.trace->next_sibling,
                                 .generation = node.generation};
    }
    // Push first child onto stack
    if (node.trace->first_child) {
      stack[top++] = TraceKeeper{.trace = node.trace->first_child,
                                 .generation = node.generation + 1};
    }
    if (node.generation != 0) {
      left_pad(buf, ' ', (node.generation - 1) * gap, total_written);
      total_written += (node.generation - 1) * gap;
    }
    left_pad(buf, '-', node.generation * gap, total_written);
    total_written += node.generation * gap;
    buf[total_written] = 0;
    program_ctxt->logger->log_debug(TRACE_TAG, "%sTrace: [%s]", buf,
                                    node.trace->name);
    total_written = 0;
    if (node.generation != 0) {

      left_pad(buf, ' ', (node.generation * 2 - 1) * gap, total_written);

      total_written += (node.generation * 2 - 1) * gap;
      buf[total_written] = 0;
    }
    program_ctxt->logger->log_debug(TRACE_TAG, "%s|Start: [%lu ns]", buf,
                                    node.trace->start_time_ns);
    program_ctxt->logger->log_debug(TRACE_TAG, "%s|Duration: [%lu ns]", buf,
                                    node.trace->duration_ns);
    program_ctxt->logger->log_debug(TRACE_TAG, "%s|Actual Duration: [%lu ns]",
                                    buf, node.trace->actual_duration_ns);
    total_written = 0;
  }
}

TraceHandler::TraceHandler(AllocatorInterface *trace_allocator,
                           uint64_t max_traces)
    : trace_allocator(trace_allocator), max_traces(max_traces) {}

Trace *TraceHandler::get_trace() {
  auto res = this->trace_allocator->allocate(sizeof(Trace));
  if (!res.has_value()) {
    program_ctxt->logger->log_err(
        TRACE_ERROR_TAG, "Trace handler failed to allocate new trace: [%s]",
        custom_strerror(res.error()));
    safe_shutdown(res.error());
  }
  auto trace_ptr = (Trace *)res.value();
  return trace_ptr;
}
void TraceHandler::clear_trace(Trace *root) {
  if (!root)
    return;
  // Use an explicit stack to avoid recursion depth issues
  Trace *stack[this->max_traces];
  int top = 0;
  stack[top++] = root;

  while (top > 0) {
    Trace *node = stack[--top];

    // Push all siblings onto stack
    if (node->next_sibling) {
      stack[top++] = node->next_sibling;
    }
    // Push first child onto stack
    if (node->first_child) {
      stack[top++] = node->first_child;
    }
    program_ctxt->logger->log_debug(TRACE_ERROR_TAG, "Cleared trace: [%s]",
                                    node->name);
    auto res = this->trace_allocator->free(node);
    if (!res.has_value()) {
      program_ctxt->logger->log_err(TRACE_ERROR_TAG,
                                    "Failed to clear trace, err: [%s]",
                                    custom_strerror(res.error()));
      safe_shutdown(res.error());
    }
  }
}
