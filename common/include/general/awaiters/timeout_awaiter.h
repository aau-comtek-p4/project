#ifndef TIMEOUT_AWAITER_H
#define TIMEOUT_AWAITER_H
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include <coroutine>
#include <cstdint>
#include <expected>
template <typename T> struct TimeoutAwaiter {
  T routine;
  uint64_t timeout;
  using promise_type = T::promise_type;
  using return_type = std::expected<typename T::value_type, int>;
  std::coroutine_handle<promise_type> routine_handler;
  DeadlineIndexKeeper *deadline_index;
  TimeoutAwaiter(T &&routine, uint64_t timeout)
      : routine(std::move(routine)), timeout(timeout) {}

  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    this->routine_handler = routine.handle;
    this->routine_handler.promise().continuation = h;
    spawn(std::move(routine));
    auto res = program_deadline_keeper->add_deadline(h, this->timeout);
    if (!res.has_value()) {
      program_logger->log_err(COROUTINE_TAG,
                              "Failed to add deadline, error: [%s]",
                              custom_strerror(res.error()));
    }
    this->deadline_index = res.value();
  }

  return_type await_resume() {
    if (this->routine_handler.done()) {
      this->deadline_index->cancelled = true;
      return this->routine_handler.promise().result;
    }
    this->routine_handler.promise().cancelled = true;
    return std::unexpected(TimeoutError::OPERATION_TIMEOUT);
  }
};
template <typename T>
TimeoutAwaiter<T> run_with_timeout(T &&routine, uint64_t timeout) {
  return TimeoutAwaiter<T>(std::move(routine), timeout);
}

#endif
