#ifndef JOB_H
#define JOB_H
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/misc/errors.h"
#include <coroutine>
#include <expected>
#include <system_error>
template <typename T>
std::expected<void, int> spawn(EventloopInterface *loop, T &&routine) {

  std::coroutine_handle<> handle = routine.get_handle();
  auto ptr = (T *)loop->allocate(sizeof(T));
  ptr[0] = std::move(routine);
  auto res = loop->enque_staging(std::move(handle));
  if (res.has_value()) {
    return {};
  }
  tl_logger->log_err(EVENT_LOOP_ERR_TAG,
                     "Failed to spawn co routine, received error: [%s]",
                     custom_strerror(res.error()));
  return std::unexpected(res.error());
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
Job::~Job() {
  if (this->handle) {
    handle.destroy();
  }
}
Job::Job(Job &&other) : handle(other.handle) {
  other.handle = nullptr; // prevent double destroy
}

Job &Job::operator=(Job &&other) {
  if (this != &other) {
    if (handle)
      handle.destroy();
    handle = other.handle;
    other.handle = nullptr;
  }
  return *this;
}

struct Job::promise_type {
  int error_type = NULL;
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
    tl_logger->log_err(COROUTINE_ERR_TAG, "Job received unexpected error: [%s]",
                       custom_strerror(error));
    this->error_type = error;
  }

  std::suspend_always initial_suspend() { return {}; }
  std::suspend_never final_suspend() noexcept { return {}; }
  void *operator new(size_t n) {
    auto res = tl_coroutine_frame_allocator->allocate(n);
    if (!res.has_value()) {
      tl_logger->log_err(
          COROUTINE_ERR_TAG,
          "Failed to allocate space for new task, got error: [%s]",
          custom_strerror(res.error()));
      safe_shutdown(res.error());
    }
    tl_logger->log_info(
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
                       "Job failed to free itself via frame "
                       "allocator, got error: [%s]",
                       custom_strerror(res.error()));

    safe_shutdown(res.error());
  }
};

#endif
