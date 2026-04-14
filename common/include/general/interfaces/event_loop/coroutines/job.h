#ifndef JOB_H
#define JOB_H
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <algorithm>
#include <cerrno>
#include <cinttypes>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <system_error>
template <typename T> std::expected<void, ErrorWrapper> spawn(T &&routine) {

  auto typed_handle = routine.handle;

  typed_handle.promise().ctxt.spawned = true;
  std::coroutine_handle<> handle = typed_handle;
  auto enqueue_res = program_ctxt->loop->enque_staging(std::move(handle));
  if (enqueue_res.has_value()) {
    return {};
  }
  return std::unexpected(enqueue_res.error());
}

template <typename T>
std::expected<void, ErrorWrapper> spawn_future(T &&routine,
                                               uint64_t future_tick_offset) {
  auto typed_handle = routine.handle;
  typed_handle.promise().ctxt.spawned = true;
  std::coroutine_handle<> handle = typed_handle;

  auto set_future_res =
      program_ctxt->loop->set_future(std::move(handle), future_tick_offset);

  if (set_future_res.has_value()) {
    return {};
  }
  return std::unexpected(set_future_res.error());
}

class Job {
public:
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  handle_type handle;
  explicit Job(handle_type h);
  ~Job();

  Job(const Job &) = delete;
  Job(Job &&other);
  Job &operator=(Job &&other);
};

struct Job::promise_type : public shared_promise_type {
  promise_type() {};

  template <typename A> auto await_transform(A &&awaiter) {
    return TraceAwaiter<A>{std::forward<A>(awaiter), &this->ctxt};
  }

  auto get_return_object() {
    auto h = handle_type::from_promise(*this);
    this->ctxt.handle = h;
    program_ctxt->metrics->document_metric(MetricType::COROUTINE_CREATED);
    this->ctxt.id =
        program_ctxt->metrics->get_metric(MetricType::COROUTINE_CREATED);
    this->ctxt.trace.id = this->ctxt.id;

    return Job(h);
  }

  void return_void() {}

  void unhandled_exception() {
    ErrorWrapper error =
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = errno};
    safe_shutdown(error);
  }

  std::suspend_always initial_suspend() { return {}; }
  std::suspend_never final_suspend() noexcept {
    this->ctxt.trace.suspend_trace();
    this->ctxt.trace.print();
    uint64_t parent_id =
        this->ctxt.parent_ctxt ? this->ctxt.parent_ctxt->name_id : 0;
    program_ctxt->logger->log_entry(logging::log_coroutine_finished(
        this->ctxt.name_id, parent_id, this->ctxt.trace.actual_duration_ns,
        this->ctxt.trace.id));
    return {};
  }
  void *operator new(size_t n) {
    auto res = program_ctxt->frame_allocator->allocate(n);
    if (!res.has_value()) {
      safe_shutdown(res.error());
    }
    return res.value();
  }

  void operator delete(void *ptr) {
    auto res = program_ctxt->frame_allocator->free(ptr);
    if (res.has_value()) {
      program_ctxt->metrics->document_metric(MetricType::COROUTINES_FREED);
      return;
    }
    safe_shutdown(res.error());
  }
};

#endif
