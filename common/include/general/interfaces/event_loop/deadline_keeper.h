#ifndef DEADLINE_KEEPER_H

#define DEADLINE_KEEPER_H

#include "general/interfaces/event_loop/co_routine.h"
#include "general/misc/errors.h"
#include <coroutine>
#include <cstdint>
#include <expected>

#define DEADLINE_TAG "DEADLINE"
#define DEADLINE_ERROR_TAG "DEADLINE ERROR"
#define DEADLINE_LOGGING 0

struct Deadline {
  std::coroutine_handle<> handle;
  uint64_t absolute_deadline_ms;
};

class DeadlineStorageInterface {
public:
  virtual std::expected<void, ErrorWrapper>
  add_deadline(std::coroutine_handle<> handle, uint64_t time_ms) = 0;
  virtual std::expected<void, ErrorWrapper> enforce_deadlines() = 0;
};

#endif
