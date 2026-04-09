#ifndef DEADLINE_KEEPER_H

#define DEADLINE_KEEPER_H

#include "general/interfaces/event_loop/co_routine.h"
#include "general/misc/errors.h"
#include <coroutine>
#include <cstdint>
#include <expected>

#define DEADLINE_TAG "DEADLINE"
#define DEADLINE_ERROR_TAG "DEADLINE ERROR"

struct Deadline {
  std::coroutine_handle<> handle;
  uint64_t deadline_tick;
  shared_promise_type *promise_type;
};

class DeadlineStorageInterface {
public:
  virtual std::expected<void, ErrorWrapper>
  add_deadline(std::coroutine_handle<> handle,
               shared_promise_type *promise_type, uint64_t remaining_tick) = 0;
  virtual std::expected<void, ErrorWrapper> enforce_deadlines() = 0;
};

#endif
