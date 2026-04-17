#ifndef SLEEP_FOR_H
#define SLEEP_FOR_H

#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/trace.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"

#include <algorithm>
#include <coroutine>
#include <cstdint>
#include <cstdio>

Task<int> sleep_for_routine();
struct SkipAwaiter {
  uint64_t sleep_time_ms;
  SkipAwaiter(uint64_t sleep_time_ms) : sleep_time_ms(sleep_time_ms) {}
  bool await_ready() { return false; }
  template <typename Promise>
  void await_suspend(std::coroutine_handle<Promise> h) {
    auto sleeper = sleep_for_routine();

    sleeper.handle.promise().ctxt.parent_ctxt = &h.promise().ctxt;

    sleeper.handle.promise().ctxt.trace.add_time(this->sleep_time_ms *
                                                 NS_PR_MS);
    sleeper.handle.promise().ctxt.trace.parent_id =
        sleeper.handle.promise().ctxt.parent_ctxt->trace.id;
    auto res = spawn_future(std::move(sleeper), this->sleep_time_ms);
    if (!res.has_value()) {
      safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
    }
  }
  void await_resume() {};
};

SkipAwaiter sleep_for(uint64_t sleep_time_ms);
#endif
