#ifndef TASK_H
#define TASK_H

#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cassert>
#include <cerrno>
#include <cinttypes>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <expected>
#include <system_error>
#include <utility>

template <typename T> class Task {
public:
  using value_type = T;
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  handle_type handle;
  explicit Task(handle_type h) : handle(h) {};
  ~Task();
  bool await_ready();

  template <typename U>
  std::coroutine_handle<> await_suspend(std::coroutine_handle<U> caller);

  T await_resume();

  Task(const Task &) = delete;
  Task(Task &&other);
  Task &operator=(Task &&other);
};
template <typename T>

bool Task<T>::await_ready() {
  if (this->handle.done()) {
  }

  return this->handle.done();
}

template <typename T>
template <typename U>
std::coroutine_handle<>
Task<T>::await_suspend(std::coroutine_handle<U> caller) {
  this->handle.promise().ctxt.parent_ctxt = &caller.promise().ctxt;
  return this->handle;
}

template <typename T> T Task<T>::await_resume() {
  return std::move(this->handle.promise().result);
}
template <typename T> Task<T>::~Task() {
  if (this->handle) {
    this->handle.destroy();
  }
}
template <typename T> Task<T>::Task(Task &&other) : handle(other.handle) {
  other.handle = nullptr; // prevent double destroy
}

template <typename T> Task<T> &Task<T>::operator=(Task &&other) {
  if (this != &other) {
    if (handle)
      handle.destroy();
    handle = other.handle;
    other.handle = nullptr;
  }
  return *this;
}
class IOAwaitInterface;

template <typename T>
struct Task<T>::promise_type : public shared_promise_type {
  T result;

  promise_type() {};
  struct FinalAwaiter {
    bool await_ready() noexcept { return false; }
    void await_suspend(
        std::coroutine_handle<Task<T>::promise_type> own_handler) noexcept {
      CoRoutineCtxt *own_ctxt = &own_handler.promise().ctxt;
      CoRoutineCtxt *parent_ctxt = own_ctxt->parent_ctxt;
      if (parent_ctxt && !own_ctxt->cancelled) {
        auto res = program_ctxt->loop->enque_staging(parent_ctxt->handle);
        if (!res.has_value()) {
          program_ctxt->logger->log_err(
              COROUTINE_ERR_TAG,
              "Task id [%" PRIu64
              "] failed to enque continuation, received error [%s]",
              own_handler.promise().ctxt.id, custom_strerror(res.error()));
          safe_shutdown(res.error());
        }
      }
      if (own_ctxt->self_cancellation) {
        void *gen_alloc = own_ctxt->self_cancellation;
        auto res = program_ctxt->loop->free(gen_alloc);
        program_ctxt->logger->log_debug(COROUTINE_TAG,
                                        "Task freeing generator");
        if (!res.has_value()) {
          program_ctxt->logger->log_err(COROUTINE_ERR_TAG,
                                        "Task failed to free generator");
          safe_shutdown(res.error());
        }
      }
      own_handler.destroy();

      return;
    }
    void await_resume() noexcept {}
  };
  auto get_return_object() {
    auto h = handle_type::from_promise(*this);
    this->ctxt.handle = h;
    this->ctxt.id =
        program_ctxt->metrics->get_metric(MetricType::TOTAL_COROUTINE);
    program_ctxt->metrics->document_metric(MetricType::TOTAL_COROUTINE);
    return Task<T>(h);
  }

  void return_value(T val) { this->result = val; }

  void unhandled_exception() {
    ErrorWrapper error{.tag = ErrorWrapper::ERRNO, .error = errno};
    program_ctxt->logger->log_err(COROUTINE_ERR_TAG,
                                  "Task received unexpected error: [%s]",
                                  custom_strerror(error));
    safe_shutdown(error);
  }

  std::suspend_always initial_suspend() { return {}; }
  FinalAwaiter final_suspend() noexcept { return {}; }
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
        COROUTINE_TAG,
        "Created new task id [%" PRIu64 "], space required: [%" PRIu64
        "] bytes",
        program_ctxt->metrics->get_metric(MetricType::TOTAL_COROUTINE),
        (uint64_t)n);
    return res.value();
  }

  void operator delete(void *ptr) {

    auto res = program_ctxt->frame_allocator->free(ptr);
    if (res.has_value()) {
      program_ctxt->metrics->document_metric(MetricType::COROUTINES_FREED);
      return;
    }
    program_ctxt->logger->log_err(COROUTINE_ERR_TAG,
                                  "Task failed to free itself via frame "
                                  "allocator, got error: [%s]",
                                  custom_strerror(res.error()));
    safe_shutdown(res.error());
  }
};

#endif
