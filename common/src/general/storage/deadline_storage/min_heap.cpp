
#include "general/common.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/deadline_storage/min_heap_storage.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <utility>

DeadlineMinHeap::DeadlineMinHeap(Deadline *deadline_buffer, size_t buffer_size,
                                 AllocatorInterface *deadline_index_allocator)
    : deadline_index_allocator(deadline_index_allocator),
      heap_buffer(deadline_buffer), buffer_size(buffer_size) {}

std::expected<DeadlineIndexKeeper *, int>
DeadlineMinHeap::add_deadline(std::coroutine_handle<> handle,
                              uint64_t deadline) {
  if (this->element_amount >= this->buffer_size) {

    program_logger->log_warning(DEADLINE_TAG,
                                "Attempt to add deadline to full buffer");
    return std::unexpected(CapacityError::INSUFFICIENT_SPACE);
  }
  auto res =
      this->deadline_index_allocator->allocate(sizeof(DeadlineIndexKeeper));
  if (!res.has_value()) {
    program_logger->log_err(
        DEADLINE_TAG, "Error allocating space for deadline index, error: [%s]",
        custom_strerror(res.error()));
    return std::unexpected(res.error());
  }
  auto deadline_index = (DeadlineIndexKeeper *)res.value();
  deadline_index->cancelled = false;

  Deadline new_deadline{
      .handle = handle,
      .deadline_ms = deadline + program_clock->rt_since_start() / NS_PR_MS,
      .deadline_index = deadline_index};
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
  program_logger->log_debug(DEADLINE_TAG, "Added deadline with ms: [%lu]",
                            new_deadline.deadline_ms);

  return deadline_index;
}
std::expected<void, int> DeadlineMinHeap::enforce_deadlines() {
  if (this->element_amount == 0) {
    program_logger->log_debug(DEADLINE_TAG, "No deadlines stored to enforce");
    return {};
  }
  uint64_t current_time =
      (program_clock->rt_since_start() + program_clock->get_time_pr_tick()) /
      NS_PR_MS;

  program_logger->log_debug(DEADLINE_TAG,
                            "Current time: [%lu], smallest deadline: [%lu]",
                            current_time, this->heap_buffer[0].deadline_ms);
  while (this->heap_buffer[0].deadline_ms <= current_time &&
         this->element_amount != 0) {
    if (!this->heap_buffer[0].deadline_index->cancelled) {
      program_logger->log_debug(DEADLINE_TAG,
                                "Enqued handler with deadline: [%lu]",
                                this->heap_buffer[0].deadline_ms);
      auto res = program_loop->enque_staging(this->heap_buffer[0].handle);
      if (!res.has_value()) {
        program_logger->log_warning(
            DEADLINE_TAG,
            "Failed to enqueue handler with deadline: [%lu], error: [%s]",
            this->heap_buffer[0].deadline_ms, custom_strerror(res.error()));
        return std::unexpected(res.error());
      }

      res = this->deadline_index_allocator->free(
          this->heap_buffer[0].deadline_index);
      if (!res.has_value()) {
        program_logger->log_err(DEADLINE_TAG,
                                "Failed to free deadline index, error: [%s]",
                                custom_strerror(res.error()));
        return std::unexpected(res.error());
      }
    } else {
      program_logger->log_debug(DEADLINE_TAG, "Deadline was cancelled");
      auto res = this->deadline_index_allocator->free(
          this->heap_buffer[0].deadline_index);
      if (!res.has_value()) {
        program_logger->log_err(DEADLINE_TAG,
                                "Failed to free deadline index, error: [%s]",
                                custom_strerror(res.error()));
        return std::unexpected(res.error());
      }
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
