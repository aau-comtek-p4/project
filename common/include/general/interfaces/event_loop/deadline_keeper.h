#ifndef DEADLINE_KEEPER_H

#define DEADLINE_KEEPER_H

#include "general/interfaces/event_loop/co_routine.h"
#include "general/misc/errors.h"
#include <coroutine>
#include <cstdint>
#include <expected>
#include <optional>

#define DEADLINE_TAG "DEADLINE"
#define DEADLINE_ERROR_TAG "DEADLINE ERROR"

struct Deadline {
  std::coroutine_handle<> handle;
  uint64_t absolute_deadline_ns;
};

class DeadlineStorageInterface {
public:
  virtual std::expected<void, ErrorWrapper>
  add_deadline(std::coroutine_handle<> handle, uint64_t time_ms) = 0;
  virtual std::expected<void, ErrorWrapper> enforce_deadlines() = 0;
  virtual std::optional<uint64_t> get_smallest_deadline_ns() = 0;
};

#endif
