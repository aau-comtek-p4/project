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
