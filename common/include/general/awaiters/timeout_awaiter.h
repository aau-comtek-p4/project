#ifndef TIMEOUT_AWAITER_H
#define TIMEOUT_AWAITER_H
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include <coroutine>
#include <cstdint>
#include <expected>
template <typename T> struct TimeoutAwaiter {
  T routine;
  uint64_t timeout_tick;
  using promise_type = T::promise_type;
  using return_type = T::value_type;
  std::coroutine_handle<promise_type> routine_handler;
  DeadlineIndexKeeper *deadline_index;
  TimeoutAwaiter(T &&routine, uint64_t timeout_tick)
      : routine(std::move(routine)), timeout_tick(timeout_tick) {}

  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    this->routine_handler = routine.handle;
    this->routine_handler.promise().continuation = h;
    spawn(std::move(routine));
    auto res = program_ctxt->deadline_tracker->add_deadline(
        h, &this->routine_handler.promise(), this->timeout_tick);
    if (!res.has_value()) {
      program_ctxt->logger->log_err(COROUTINE_TAG,
                                    "Failed to add deadline, error: [%s]",
                                    custom_strerror(res.error()));
    }
    this->deadline_index = res.value();
  }

  return_type await_resume() {
    if (this->routine_handler.done()) {
      this->deadline_index->cancelled = true;
      return_type res = this->routine_handler.promise().result;
      return res;
    }
    this->routine_handler.promise().cancelled = true;
    return std::unexpected(ErrorWrapper{.tag = ErrorWrapper::CUSTOM,
                                        .error = TimeoutError::TIMEOUT});
  }
};
template <typename T>
TimeoutAwaiter<T> run_with_timeout(T &&routine, uint64_t timeout_tick) {

  return TimeoutAwaiter<T>(std::move(routine), timeout_tick);
}

#endif
