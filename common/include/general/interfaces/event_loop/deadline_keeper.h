#ifndef DEADLINE_KEEPER_H
#define DEADLINE_KEEPER_H

#include <coroutine>
#include <cstdint>
#include <expected>

#define DEADLINE_TAG "DEADLINE"
#define DEADLINE_ERROR_TAG "DEADLINE ERROR"

struct Deadline {
  std::coroutine_handle<> handle;
  uint64_t deadline_ms;
};

class DeadlineStorageInterface {
public:
  virtual std::expected<void, int> add_deadline(std::coroutine_handle<> handle,
                                                uint64_t remaining_tick) = 0;
  virtual std::expected<void, int> enforce_deadlines() = 0;
};

#endif
