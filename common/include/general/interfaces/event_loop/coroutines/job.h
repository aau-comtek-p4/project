#ifndef JOB_H
#define JOB_H
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <algorithm>
#include <coroutine>
#include <cstdint>
#include <expected>
#include <system_error>
template <typename T> std::expected<void, int> spawn(T &&routine) {
  auto typed_handle = routine.handle;

  auto res = program_loop->allocate(sizeof(T));
  if (!res.has_value()) {
    program_logger->log_err(
        EVENT_LOOP_ERR_TAG,
        "In spawn failed to allocate enough space for coroutine "
        "generator, error: [%s]",
        custom_strerror(res.error()));
    return std::unexpected(res.error());
  }
  T *ptr = new (res.value()) T(std::move(routine));

  typed_handle.promise().self_cancellation = ptr;
  std::coroutine_handle<> handle = typed_handle;
  auto enqueue_res = program_loop->enque_staging(std::move(handle));
  if (enqueue_res.has_value()) {
    return {};
  }
  program_logger->log_err(EVENT_LOOP_ERR_TAG,
                          "Failed to spawn co routine, received error: [%s]",
                          custom_strerror(enqueue_res.error()));
  return std::unexpected(enqueue_res.error());
}

template <typename T>
std::expected<void, int> spawn_future(T &&routine,
                                      uint64_t future_tick_offset) {
  auto typed_handle = routine.handle;

  auto res = program_loop->allocate(sizeof(T));
  if (!res.has_value()) {
    program_logger->log_err(
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
      program_loop->set_future(std::move(handle), future_tick_offset);
  if (set_future_res.has_value()) {
    return {};
  }
  program_logger->log_err(
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
  int error_type = 0;
  size_t id;
  promise_type() {
    id = total_coroutine_counter;
    total_coroutine_counter += 1;
  };
  auto get_return_object() { return Job(handle_type::from_promise(*this)); }

  void return_void() {}

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
    program_logger->log_err(COROUTINE_ERR_TAG,
                            "Job received unexpected error: [%s]",
                            custom_strerror(error));
    this->error_type = error;
    safe_shutdown(error);
  }

  std::suspend_always initial_suspend() { return {}; }
  std::suspend_never final_suspend() noexcept {
    void *gen_alloc = this->self_cancellation;
    if (this->self_cancellation) {
      auto res = program_loop->free(gen_alloc);
      program_logger->log_debug(COROUTINE_TAG, "Freeing generator");
      if (!res.has_value()) {

        program_logger->log_err(COROUTINE_ERR_TAG, "Failed to free generator");
        safe_shutdown(1);
      }
    }

    return {};
  }
  void *operator new(size_t n) {
    auto res = program_coroutine_frame_allocator->allocate(n);
    if (!res.has_value()) {
      program_logger->log_err(
          COROUTINE_ERR_TAG,
          "Failed to allocate space for new task, got error: [%s]",
          custom_strerror(res.error()));
      safe_shutdown(res.error());
    }
    program_logger->log_debug(
        COROUTINE_TAG, "Created job task id [%lu], space required: [%lu] bytes",
        total_coroutine_counter, n);
    return res.value();
  }

  void operator delete(void *ptr) {
    auto res = program_coroutine_frame_allocator->free(ptr);
    if (res.has_value()) {
      return;
    }
    program_logger->log_err(COROUTINE_ERR_TAG,
                            "Job failed to free itself via frame "
                            "allocator, got error: [%s]",
                            custom_strerror(res.error()));

    safe_shutdown(res.error());
  }
};

#endif
