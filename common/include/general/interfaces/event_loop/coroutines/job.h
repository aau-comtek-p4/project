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
#include <coroutine>
#include <cstdint>
#include <expected>
#include <system_error>
template <typename T> std::expected<void, ErrorWrapper> spawn(T &&routine) {
  auto typed_handle = routine.handle;

  auto res = program_ctxt->loop->allocate(sizeof(T));
  if (!res.has_value()) {
    program_ctxt->logger->log_err(
        EVENT_LOOP_ERR_TAG,
        "In spawn failed to allocate enough space for coroutine "
        "generator, error: [%s]",
        custom_strerror(res.error()));
    return std::unexpected(res.error());
  }
  T *ptr = new (res.value()) T(std::move(routine));

  typed_handle.promise().self_cancellation = ptr;
  std::coroutine_handle<> handle = typed_handle;
  auto enqueue_res = program_ctxt->loop->enque_staging(std::move(handle));
  if (enqueue_res.has_value()) {
    return {};
  }
  program_ctxt->logger->log_err(
      EVENT_LOOP_ERR_TAG, "Failed to spawn co routine, received error: [%s]",
      custom_strerror(enqueue_res.error()));
  return std::unexpected(enqueue_res.error());
}

template <typename T>
std::expected<void, ErrorWrapper> spawn_future(T &&routine,
                                               uint64_t future_tick_offset) {
  auto typed_handle = routine.handle;

  auto res = program_ctxt->loop->allocate(sizeof(T));
  if (!res.has_value()) {
    program_ctxt->logger->log_err(
        EVENT_LOOP_ERR_TAG,
        "In set future failed to allocate enough space for coroutine "
        "generator, error: [%s]",
        custom_strerror(res.error()));
    return std::unexpected(res.error());
  }

  T *ptr = new (res.value()) T(std::move(routine));
  typed_handle.promise().self_cancellation = ptr;
  std::coroutine_handle<> handle = typed_handle;
  auto set_future_res =
      program_ctxt->loop->set_future(std::move(handle), future_tick_offset);
  if (set_future_res.has_value()) {
    return {};
  }
  program_ctxt->logger->log_err(
      EVENT_LOOP_ERR_TAG,
      "Failed to set future co routine, received error: [%s]",
      custom_strerror(set_future_res.error()));
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
  size_t id;
  promise_type() {
    id = program_ctxt->metrics->get_metric(MetricType::TOTAL_COROUTINE);
    program_ctxt->metrics->document_metric(MetricType::TOTAL_COROUTINE);
  };
  auto get_return_object() { return Job(handle_type::from_promise(*this)); }

  void return_void() {}

  void unhandled_exception() {
    ErrorWrapper error =
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = errno};
    program_ctxt->logger->log_err(COROUTINE_ERR_TAG,
                                  "Job received unexpected error: [%s]",
                                  custom_strerror(error));
    safe_shutdown(error);
  }

  std::suspend_always initial_suspend() { return {}; }
  std::suspend_never final_suspend() noexcept {
    void *gen_alloc = this->self_cancellation;
    if (this->self_cancellation) {
      auto res = program_ctxt->loop->free(gen_alloc);
      program_ctxt->logger->log_debug(COROUTINE_TAG, "Job freeing generator");
      if (!res.has_value()) {

        program_ctxt->logger->log_err(COROUTINE_ERR_TAG,
                                      "Job failed to free generator");
        safe_shutdown(res.error());
      }
    }

    return {};
  }
  void *operator new(size_t n) {
    auto res = program_ctxt->frame_allocator->allocate(n);
    if (!res.has_value()) {
      program_ctxt->logger->log_err(
          COROUTINE_ERR_TAG,
          "Failed to allocate space for new task, got error: [%s]",
          custom_strerror(res.error()));
      safe_shutdown(res.error());
    }
    program_ctxt->logger->log_debug(
        COROUTINE_TAG, "Created job task id [%lu], space required: [%lu] bytes",
        program_ctxt->metrics->get_metric(MetricType::TOTAL_COROUTINE), n);
    return res.value();
  }

  void operator delete(void *ptr) {
    auto res = program_ctxt->frame_allocator->free(ptr);
    if (res.has_value()) {
      program_ctxt->metrics->document_metric(MetricType::COROUTINES_FREED);
      return;
    }
    program_ctxt->logger->log_err(COROUTINE_ERR_TAG,
                                  "Job failed to free itself via frame "
                                  "allocator, got error: [%s]",
                                  custom_strerror(res.error()));

    safe_shutdown(res.error());
  }
};

#endif
