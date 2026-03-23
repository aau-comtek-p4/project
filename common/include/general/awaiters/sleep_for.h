#ifndef SLEEP_FOR_H
#define SLEEP_FOR_H

#include "general/common.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include <coroutine>
#include <cstdint>

struct SkipAwaiter {
  uint64_t timeout;
  SkipAwaiter(uint64_t timeout) : timeout(timeout) {}
  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    auto _ = program_deadline_keeper->add_deadline(h, timeout);
  }
  void await_resume() {};
};

SkipAwaiter sleep_for(uint64_t timeout);
#endif
