#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/misc/errors.h"
#include <cstdio>
#include <system_error>
Job::Job(handle_type h) : handle(h) {}

Job::~Job() {
  if (this->handle && !this->handle.promise().ctxt.spawned) {
    this->handle.destroy();
    this->handle = nullptr;
  }
}
Job::Job(Job &&other) : handle(other.handle) {
  other.handle = nullptr; // prevent double destroy
}

Job &Job::operator=(Job &&other) {
  if (this != &other) {
    if (this->handle)
      this->handle.destroy();
    this->handle = other.handle;
    other.handle = nullptr;
  }
  return *this;
}
