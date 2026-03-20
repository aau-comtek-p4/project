
#include "general/common.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/deadline_storage/min_heap_storage.h"
#include "general/interfaces/utility/clock.h"
#include "general/misc/errors.h"
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <utility>

DeadlineMinHeap::DeadlineMinHeap(Deadline *deadline_buffer, size_t buffer_size)
    : heap_buffer(deadline_buffer), buffer_size(buffer_size) {}

std::expected<void, int>
DeadlineMinHeap::add_deadline(std::coroutine_handle<> handle,
                              uint64_t deadline) {
  if (this->element_amount >= this->buffer_size) {
    tl_logger->log_warning(DEADLINE_TAG,
                           "Attempt to add deadline to full buffer");
    return std::unexpected(CapacityError::INSUFFICIENT_SPACE);
  }

  Deadline new_deadline{.handle = handle,
                        .deadline_ms =
                            deadline + tl_clock->rt_since_start() / NS_PR_MS};
  if (this->element_amount == 0) {
    this->heap_buffer[0] = new_deadline;
  }

  size_t index = this->element_amount;
  this->heap_buffer[index] = new_deadline;

  while (index > 0 && this->heap_buffer[(index - 1) / 2].deadline_ms >
                          this->heap_buffer[index].deadline_ms) {
    std::swap(this->heap_buffer[index], this->heap_buffer[(index - 1) / 2]);
    index = (index - 1) / 2;
  }
  this->element_amount += 1;
  tl_logger->log_debug(DEADLINE_TAG, "Added deadline with ms: [%lu]",
                       new_deadline.deadline_ms);

  return {};
}
std::expected<void, int> DeadlineMinHeap::enforce_deadlines() {
  if (this->element_amount == 0) {
    tl_logger->log_debug(DEADLINE_TAG, "No deadlines stored to enforce");
    return {};
  }
  uint64_t current_time =
      (tl_clock->rt_since_start() + tl_clock->get_time_pr_tick()) / NS_PR_MS;

  tl_logger->log_debug(DEADLINE_TAG,
                       "Current time: [%lu], smallest deadline: [%lu]",
                       current_time, this->heap_buffer[0].deadline_ms);
  while (this->heap_buffer[0].deadline_ms <= current_time &&
         this->element_amount != 0) {
    tl_logger->log_debug(DEADLINE_TAG, "Enqued handler with deadline: [%lu]",
                         this->heap_buffer[0].deadline_ms);
    auto res = tl_loop->enque_staging(this->heap_buffer[0].handle);
    if (!res.has_value()) {
      tl_logger->log_warning(
          DEADLINE_TAG,
          "Failed to enqueue handler with deadline: [%lu], error: [%s]",
          this->heap_buffer[0].deadline_ms, custom_strerror(res.error()));
      return std::unexpected(res.error());
    }
    std::swap(this->heap_buffer[0],
              this->heap_buffer[this->element_amount - 1]);
    this->element_amount -= 1;
    size_t index = 0;
    while (index < this->element_amount) {
      size_t smallest = index;
      size_t left_index = 2 * index + 1;
      size_t right_index = 2 * index + 2;
      if (left_index < this->element_amount &&
          this->heap_buffer[left_index].deadline_ms <
              this->heap_buffer[smallest].deadline_ms) {
        smallest = left_index;
      }

      if (right_index < this->element_amount &&
          this->heap_buffer[right_index].deadline_ms <
              this->heap_buffer[smallest].deadline_ms) {
        smallest = right_index;
      }
      if (smallest != index) {
        std::swap(this->heap_buffer[index], this->heap_buffer[smallest]);
        index = smallest;
      } else {
        break;
      }
    }
  }
  return {};
}
