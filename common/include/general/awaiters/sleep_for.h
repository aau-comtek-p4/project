#ifndef SLEEP_FOR_H
#define SLEEP_FOR_H

#include "general/common.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/trace.h"
#include "general/misc/context.h"

#include <coroutine>
#include <cstdint>

struct SkipAwaiter {
  uint64_t timeout;
  SkipAwaiter(uint64_t timeout) : timeout(timeout) {}
  bool await_ready() { return false; }
  template <typename Promise>
  void await_suspend(std::coroutine_handle<Promise> h) {
    uint64_t wall_sleep_time =
        this->timeout * program_ctxt->clock->ms_pr_tick() * NS_PR_MS;
    Trace *trace = program_ctxt->trace_handler->get_trace();
    trace->start();
    trace->add_time(wall_sleep_time);
    trace->set_name("Sleep");
    trace->end();
    h.promise().ctxt.trace->append_child(trace);
    h.promise().ctxt.trace->add_time(wall_sleep_time);
    auto _ = program_ctxt->deadline_tracker->add_deadline(h, nullptr, timeout);
  }
  void await_resume() {};
};

SkipAwaiter sleep_for(uint64_t timeout_tick);
#endif
