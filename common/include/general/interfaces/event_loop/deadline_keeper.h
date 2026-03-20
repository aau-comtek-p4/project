#ifndef DEADLINE_KEEPER_H
#define DEADLINE_KEEPER_H

#include <coroutine>
#include <cstdint>
#include <expected>

#define DEADLINE_TAG "DEADLINE"
#define DEADLINE_ERROR_TAG "DEADLINE ERROR"
struct DeadlineIndexKeeper;

struct Deadline {
  std::coroutine_handle<> handle;
  uint64_t deadline_ms;
  DeadlineIndexKeeper *deadline_index;
};

struct DeadlineIndexKeeper {
  bool cancelled;
};

class DeadlineStorageInterface {
public:
  virtual std::expected<DeadlineIndexKeeper *, int>
  add_deadline(std::coroutine_handle<> handle, uint64_t remaining_tick) = 0;
  virtual std::expected<void, int> enforce_deadlines() = 0;
};

#endif
