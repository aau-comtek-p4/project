#ifndef DEADLINE_MIN_HEAP_STORAGE
#define DEADLINE_MIN_HEAP_STORAGE

#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/misc/errors.h"
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <expected>

class DeadlineMinHeap : public DeadlineStorageInterface {
private:
  Deadline *heap_buffer;
  size_t buffer_size;
  size_t element_amount;

public:
  DeadlineMinHeap(Deadline *heaper_buffer, size_t buffer_size);

  std::expected<void, ErrorWrapper>
  add_deadline(std::coroutine_handle<> handle,
               uint64_t remaining_tick) override;
  std::expected<void, ErrorWrapper> enforce_deadlines() override;
};

#endif
