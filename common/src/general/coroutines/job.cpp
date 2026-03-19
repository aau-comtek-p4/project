#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/misc/errors.h"
#include <system_error>
Job::Job(handle_type h) : handle(h) {}

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

class AwaitNothing {
public:
  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    auto res = tl_loop->enque_staging(std::move(h));
    if (res.has_value()) {
      return;
    }
    tl_logger->log_err("MAIN", "Failed to enque staging, err: [%s]",
                       custom_strerror(res.error()));
    return;
  }
  void await_resume() {}
};
Task<int> await_nothing() { co_return 1; }

Job keep_printing_boy() {
  while (true) {
    tl_logger->log_info("MAIN", "Do be printing");
    co_await await_nothing();
  }
}

Job keep_printing_boy2() {
  while (true) {
    tl_logger->log_info("MAIN", "Do be printing, but cooler");
    co_await await_nothing();
  }
}
