#ifndef TIMEOUT_AWAITER_H
#define TIMEOUT_AWAITER_H
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <coroutine>
#include <cstdint>
#include <expected>

Task<int> timeout_routine();
template <typename T> struct TimeoutAwaiter {
  T routine;
  uint64_t timeout_ms;
  using promise_type = T::promise_type;
  using return_type = T::value_type;
  std::coroutine_handle<Task<int>::promise_type> timeout_handle;
  std::coroutine_handle<promise_type> routine_handler;
  TimeoutAwaiter(T &&routine, uint64_t timeout_ms)
      : routine(std::move(routine)), timeout_ms(timeout_ms) {}

  bool await_ready() { return false; }
  template <typename Promise>
  void await_suspend(std::coroutine_handle<Promise> h) {
    this->routine_handler = routine.handle;
    this->routine_handler.promise().ctxt.parent_ctxt = &h.promise().ctxt;
    this->routine_handler.promise().ctxt.trace.parent_id =
        this->routine_handler.promise().ctxt.parent_ctxt->trace.id;
    auto res = spawn(std::move(routine));
    if (!res.has_value()) {
      safe_shutdown(res.error());
    }
    auto timeouter = timeout_routine();
    this->timeout_handle = timeouter.handle;
    this->timeout_handle.promise().ctxt.io_address =
        this->routine_handler.promise().ctxt.io_address;
    this->timeout_handle.promise().ctxt.parent_ctxt = &h.promise().ctxt;
    this->timeout_handle.promise().ctxt.trace.add_time(this->timeout_ms *
                                                       NS_PR_MS);
    this->timeout_handle.promise().ctxt.trace.parent_id =
        this->timeout_handle.promise().ctxt.parent_ctxt->trace.id;
    res = spawn_future(std::move(timeouter), this->timeout_ms);
    if (!res.has_value()) {
      safe_shutdown(res.error());
    }
  }

  std::expected<return_type, ErrorWrapper> await_resume() {
    if (this->routine_handler.done()) {
      this->timeout_handle.promise().ctxt.cancelled = true;
      this->routine_handler.promise().ctxt.trace.parent_id =
          this->routine_handler.promise().ctxt.parent_ctxt->trace.id;
      return_type res = this->routine_handler.promise().result;
      return res;
    }
    program_ctxt->metrics->document_metric(MetricType::SURPASSED_DEADLINE);
    this->routine_handler.promise().ctxt.cancelled = true;
    program_ctxt->logger->log_entry(logging::log_coroutine_timeout(
        this->routine_handler.promise().ctxt.name_id,
        this->routine_handler.promise().ctxt.parent_ctxt->name_id,
        this->routine_handler.promise().ctxt.trace.id));
    return std::unexpected(ErrorWrapper{.tag = ErrorWrapper::CUSTOM,
                                        .error = TimeoutError::TIMEOUT});
  }
};
template <typename T>
TimeoutAwaiter<T> run_with_timeout(T &&routine, uint64_t timeout_ms) {

  return TimeoutAwaiter<T>(std::move(routine), timeout_ms);
}

#endif
