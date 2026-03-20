#ifndef TASK_H
#define TASK_H

#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/storage/allocator.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <cstring>
#include <exception>
#include <expected>
#include <system_error>
#include <utility>

template <typename T> class Task {
public:
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  handle_type handle;
  explicit Task(handle_type h) : handle(h) {};
  ~Task();
  bool await_ready();

  template <typename U>
  std::coroutine_handle<> await_suspend(std::coroutine_handle<U> caller);

  std::expected<T, int> await_resume();

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
  this->handle.promise().continuation = caller;
  return this->handle;
}

template <typename T> std::expected<T, int> Task<T>::await_resume() {
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

template <typename T>
struct Task<T>::promise_type : public countable_promise_type {
  std::expected<T, int> result;
  size_t id;
  std::coroutine_handle<> continuation = nullptr;
  promise_type() {
    id = total_coroutine_counter;
    total_coroutine_counter += 1;
  };
  struct FinalAwaiter {
    bool await_ready() noexcept { return false; }
    void
    await_suspend(std::coroutine_handle<promise_type> own_handler) noexcept {
      std::coroutine_handle<> continuation_handler =
          own_handler.promise().continuation;
      if (!continuation_handler) {
        return;
      }
      auto res = tl_loop->enque_staging(continuation_handler);
      if (res.has_value()) {
        return;
      }
      tl_logger->log_err(
          COROUTINE_ERR_TAG,
          "Task id [%lu] failed to enque continuation, received error [%s]",
          own_handler.promise().id, custom_strerror(res.error()));
      own_handler.promise().result = std::unexpected(res.error());
    }
    void await_resume() noexcept {}
  };
  auto get_return_object() { return Task<T>(handle_type::from_promise(*this)); }

  template <std::convertible_to<T> From> void return_value(From &&from) {
    this->result = std::forward<From>(from);
  }

  void unhandled_exception() {
    int error;
    try {
      std::rethrow_exception(std::current_exception());
    } catch (const std::bad_alloc &) {
      error = CustomErrors::OUT_OF_MEMORY;
    } catch (const std::system_error &e) {
      error = e.code().value();
    } catch (const std::runtime_error &) {
      error = CustomErrors::HARDWARE_FAILURE;
    } catch (...) {
      error = CustomErrors::INVALID_STATE;
    }
    tl_logger->log_err(COROUTINE_ERR_TAG,
                       "Task received unexpected error: [%s]",
                       custom_strerror(error));
    this->result = std::unexpected(error);
  }

  std::suspend_always initial_suspend() { return {}; }
  FinalAwaiter final_suspend() noexcept { return {}; }
  void *operator new(size_t n) {
    auto res = tl_coroutine_frame_allocator->allocate(n);
    if (!res.has_value()) {
      tl_logger->log_err(
          COROUTINE_ERR_TAG,
          "Failed to allocate space for new task, got error: [%s]",
          custom_strerror(res.error()));
      safe_shutdown(res.error());
    }
    tl_logger->log_debug(
        COROUTINE_TAG, "Created new task id [%lu], space required: [%lu] bytes",
        total_coroutine_counter, n);
    return res.value();
  }

  void operator delete(void *ptr) {

    auto res = tl_coroutine_frame_allocator->free(ptr);
    if (res.has_value()) {
      return;
    }
    tl_logger->log_err(COROUTINE_ERR_TAG,
                       "Task failed to free itself via frame "
                       "allocator, got error: [%s]",
                       custom_strerror(res.error()));
    safe_shutdown(res.error());
  }
};

#endif
