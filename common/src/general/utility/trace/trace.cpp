#include "general/interfaces/utility/trace.h"
#include "general/common.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>
void Trace::add_time(uint64_t time_ns) { this->duration_ns += time_ns; }
void Trace::add_actual_time(uint64_t time_ns) {
  this->actual_duration_ns += time_ns;
}
void Trace::set_name(uint64_t name_id) { this->name_id = name_id; }

void Trace::start() {
  this->started = true;
  this->start_time_ns = program_ctxt->clock->rt_since_start_ns();
  this->last_suspend_ns = this->start_time_ns;
}
void Trace::end() { this->started = false; }
void Trace::suspend_trace(bool debug) {
  if (this->started) {
    uint64_t additional_time =
        program_ctxt->clock->rt_since_start_ns() - this->last_suspend_ns;
    if (debug) {
      /*
program_ctxt->logger->log_entry(
    logging::log_coroutine_suspended(this->name_id, additional_time));
        */

      program_ctxt->metrics->document_statistics_metric_metric(
          StatMetricType::METRIC_SUSPEND_TIME, additional_time);
    }

    this->duration_ns += additional_time;
    this->actual_duration_ns += additional_time;
  }
}
void Trace::suspend_trace() { this->suspend_trace(true); }
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
  if (TRACE_LOGGING) {
    program_ctxt->logger->log_entry(
        logging::log_trace_print(this->name_id, this->id, this->parent_id,
                                 this->duration_ns, this->actual_duration_ns));
  }
}
