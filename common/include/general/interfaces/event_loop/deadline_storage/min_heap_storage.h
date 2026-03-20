#ifndef DEADLINE_MIN_HEAP_STORAGE
#define DEADLINE_MIN_HEAP_STORAGE

#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/storage/allocator.h"
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <expected>

class DeadlineMinHeap : public DeadlineStorageInterface {
private:
  AllocatorInterface *deadline_index_allocator;
  Deadline *heap_buffer;
  size_t buffer_size;
  size_t element_amount;

public:
  DeadlineMinHeap(Deadline *heaper_buffer, size_t buffer_size,
                  AllocatorInterface *deadline_index_allocator);

  std::expected<DeadlineIndexKeeper *, int>
  add_deadline(std::coroutine_handle<> handle,
               uint64_t remaining_tick) override;
  std::expected<void, int> enforce_deadlines() override;
};

#endif
