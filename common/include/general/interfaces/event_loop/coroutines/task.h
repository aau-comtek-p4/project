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
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cinttypes>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <expected>
#include <system_error>
#include <type_traits>
#include <utility>

template <typename T> class Task {
public:
  using value_type = T;
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  handle_type handle;
  explicit Task(handle_type h) : handle(h) {};
  bool await_ready();
  ~Task();

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

template <typename T> Task<T>::~Task() {
  if (this->handle && !this->handle.promise().ctxt.spawned) {
    this->handle.destroy();
    this->handle = nullptr;
  }
}

template <typename T>
template <typename U>
std::coroutine_handle<>
Task<T>::await_suspend(std::coroutine_handle<U> caller) {
  this->handle.promise().ctxt.parent_ctxt = &caller.promise().ctxt;
  this->handle.promise().ctxt.trace.parent_id =
      this->handle.promise().ctxt.parent_ctxt->trace.id;
  auto _ = program_ctxt->loop->enque_staging(this->handle);
  return std::noop_coroutine();
}

template <typename T> T Task<T>::await_resume() {
  T val = std::move(this->handle.promise().result);
  this->handle.destroy();
  this->handle = nullptr;
  return val;
}
template <typename T> Task<T>::Task(Task &&other) : handle(other.handle) {
  other.handle = nullptr; // prevent double destroy
}

template <typename T> Task<T> &Task<T>::operator=(Task &&other) {
  if (this != &other) {
    if (this->handle) {
      this->handle.destroy();
    }
    this->handle = other.handle;
    other.handle = nullptr;
  }
  return *this;
}
class IOAwaitInterface;

template <typename T>
struct Task<T>::promise_type : public shared_promise_type {
  T result;

  promise_type() {};
  template <typename A> auto await_transform(A &&awaiter) {
    return TraceAwaiter<A>{std::forward<A>(awaiter), &this->ctxt};
  }
  struct FinalAwaiter {
    std::coroutine_handle<Task<T>::promise_type> handle;
    bool await_ready() noexcept { return false; }
    void await_suspend(
        std::coroutine_handle<Task<T>::promise_type> own_handler) noexcept {
      this->handle = own_handler;
      CoRoutineCtxt *own_ctxt = &own_handler.promise().ctxt;
      CoRoutineCtxt *parent_ctxt = own_ctxt->parent_ctxt;
      if (parent_ctxt && !own_ctxt->cancelled) {
        parent_ctxt->trace.add_time(own_ctxt->trace.duration_ns);
        parent_ctxt->trace.add_actual_time(own_ctxt->trace.actual_duration_ns);
        auto res = program_ctxt->loop->enque_staging(parent_ctxt->handle);
        if (!res.has_value()) {
          safe_shutdown(res.error());
        }
      }
      if (own_ctxt->cancelled) {
        own_ctxt->trace.parent_id = 0;
      }
      own_ctxt->trace.print();
      if (own_ctxt->spawned) {
        own_handler.destroy();
      }
      return;
    }
    T await_resume() noexcept {
      T val = std::move(this->handle.promise().result);
      return val;
    }
  };
  auto get_return_object() {
    auto h = handle_type::from_promise(*this);
    this->ctxt.handle = h;
    program_ctxt->metrics->document_metric(MetricType::COROUTINE_CREATED);
    this->ctxt.id =
        program_ctxt->metrics->get_metric(MetricType::COROUTINE_CREATED);
    this->ctxt.trace.id = this->ctxt.id;
    return Task<T>(h);
  }

  void return_value(T val) { this->result = val; }

  void unhandled_exception() {
    ErrorWrapper error{.tag = ErrorWrapper::ERRNO, .error = errno};
    safe_shutdown(error);
  }

  std::suspend_always initial_suspend() { return {}; }
  FinalAwaiter final_suspend() noexcept {
    this->ctxt.trace.suspend_trace();
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
