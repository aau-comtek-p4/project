#ifndef SLEEP_FOR_H
#define SLEEP_FOR_H

#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/trace.h"
#include "general/misc/context.h"

#include <algorithm>
#include <coroutine>
#include <cstdint>
#include <cstdio>

Task<int> sleep_for_routine();
struct SkipAwaiter {
  uint64_t timeout;
  SkipAwaiter(uint64_t timeout) : timeout(timeout) {}
  bool await_ready() { return false; }
  template <typename Promise>
  void await_suspend(std::coroutine_handle<Promise> h) {
    auto sleeper = sleep_for_routine();

    sleeper.handle.promise().ctxt.parent_ctxt = &h.promise().ctxt;

    sleeper.handle.promise().ctxt.trace->add_time(
        timeout * program_ctxt->clock->ms_pr_tick() * NS_PR_MS);
    sleeper.handle.promise().ctxt.trace->parent_id =
        sleeper.handle.promise().ctxt.parent_ctxt->trace->id;

    spawn_future(std::move(sleeper), this->timeout - 1);
  }
  void await_resume() {};
};

SkipAwaiter sleep_for(uint64_t timeout_tick);
#endif
