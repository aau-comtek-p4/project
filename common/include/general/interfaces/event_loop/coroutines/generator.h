
#ifndef GENERATOR_H
#define GENERATOR_H

#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include <cerrno>
#include <coroutine>
template <typename T> class Generator : public Coroutine {
public:
  using value_type = T;
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  handle_type handle;
  explicit Generator(handle_type h) : handle(h) {};

  struct NextAwaiter {
    handle_type handle;
    bool await_ready() noexcept { return this->handle.done(); }
    template <typename C>
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<C> caller) noexcept {

      this->handle.promise().ctxt.parent_ctxt = &caller.promise().ctxt;
      this->handle.promise().ctxt.trace.parent_id =
          this->handle.promise().ctxt.parent_ctxt->trace.id;
      auto res = program_ctxt->loop->enque(this->handle);
      if (!res.has_value()) {
        safe_shutdown(res.error());
      }

      return std::noop_coroutine();
    }
    T await_resume() {
      T val = std::move(this->handle.promise().value);
      return val;
    }
  };

  NextAwaiter next() { return NextAwaiter{handle}; }
  T await_resume();
};

template <typename T>
struct Generator<T>::promise_type : public shared_promise_type {
  T value;

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
    return Generator<T>(h);
  }

  void return_void() {}
  void unhandled_exception() {
    ErrorWrapper error{.error = errno, .tag = ErrorWrapper::ERRNO};
    safe_shutdown(error);
  }
  std::suspend_always yield_value(T val) noexcept {
    this->value = std::move(val);
    auto err = program_ctxt->loop->enque(this->ctxt.parent_ctxt->handle);
    if (!err.has_value()) {
      safe_shutdown(err.error());
    }
    return {};
  }

  std::suspend_always initial_suspend() { return {}; }
  std::suspend_never final_suspend() noexcept {
    this->ctxt.trace.suspend_trace();
    uint64_t parent_id =
        this->ctxt.parent_ctxt ? this->ctxt.parent_ctxt->name_id : 0;
    if (COROUTINE_LOGGING) {
      program_ctxt->logger->log_entry(logging::log_coroutine_finished(
          this->ctxt.name_id, parent_id, this->ctxt.trace.actual_duration_ns,
          this->ctxt.trace.id));
    }
    CoRoutineCtxt *parent_ctxt = this->ctxt.parent_ctxt;
    parent_ctxt->trace.add_actual_time(this->ctxt.trace.actual_duration_ns);
    auto res = program_ctxt->loop->enque(parent_ctxt->handle);
    if (!res.has_value()) {
      safe_shutdown(res.error());
    }
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
