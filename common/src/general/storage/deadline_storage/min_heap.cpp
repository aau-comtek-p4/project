
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/deadline_storage/min_heap_storage.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cassert>
#include <cinttypes>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <expected>
#include <optional>
#include <utility>

DeadlineMinHeap::DeadlineMinHeap(Deadline *deadline_buffer, size_t buffer_size)
    : heap_buffer(deadline_buffer), buffer_size(buffer_size) {}

std::expected<void, ErrorWrapper>
DeadlineMinHeap::add_deadline(std::coroutine_handle<> handle,
                              uint64_t time_ms) {
  if (this->element_amount >= this->buffer_size) {
    return std::unexpected(
        ErrorWrapper{.error = CapacityError::INSUFFICIENT_SPACE,
                     .tag = ErrorWrapper::CUSTOM});
  }

  Deadline new_deadline{
      .handle = handle,
      .absolute_deadline_ns =
          program_ctxt->clock->rt_since_start_ns() + time_ms * NS_PR_MS

  };

  if (DEADLINE_LOGGING) {
    program_ctxt->logger->log_entry(
        logging::log_new_deadline(new_deadline.absolute_deadline_ns));
  }

  if (this->element_amount == 0) {
    this->heap_buffer[0] = new_deadline;
  }

  size_t index = this->element_amount;
  this->heap_buffer[index] = new_deadline;

  while (index > 0 && this->heap_buffer[(index - 1) / 2].absolute_deadline_ns >
                          this->heap_buffer[index].absolute_deadline_ns) {
    std::swap(this->heap_buffer[index], this->heap_buffer[(index - 1) / 2]);
    index = (index - 1) / 2;
  }
  this->element_amount += 1;

  return {};
}
std::expected<void, ErrorWrapper> DeadlineMinHeap::enforce_deadlines() {
  if (this->element_amount == 0) {
    return {};
  }
  uint64_t current_time = program_ctxt->clock->rt_since_start_ns();
  while (this->heap_buffer[0].absolute_deadline_ns <= current_time &&
         this->element_amount != 0) {
    this->heap_buffer[0].handle.resume();

    std::swap(this->heap_buffer[0],
              this->heap_buffer[this->element_amount - 1]);
    this->element_amount -= 1;
    size_t index = 0;
    while (index < this->element_amount) {
      size_t smallest = index;
      size_t left_index = 2 * index + 1;
      size_t right_index = 2 * index + 2;
      if (left_index < this->element_amount &&
          this->heap_buffer[left_index].absolute_deadline_ns <
              this->heap_buffer[smallest].absolute_deadline_ns) {
        smallest = left_index;
      }

      if (right_index < this->element_amount &&
          this->heap_buffer[right_index].absolute_deadline_ns <
              this->heap_buffer[smallest].absolute_deadline_ns) {
        smallest = right_index;
      }
      if (smallest != index) {
        std::swap(this->heap_buffer[index], this->heap_buffer[smallest]);
        index = smallest;
      } else {
        break;
      }
    }
    current_time = program_ctxt->clock->rt_since_start_ns();
  }
  return {};
}

std::optional<uint64_t> DeadlineMinHeap::get_smallest_deadline_ns() {
  if (this->element_amount == 0) {
    return {};
  }
  return this->heap_buffer[0].absolute_deadline_ns;
}
