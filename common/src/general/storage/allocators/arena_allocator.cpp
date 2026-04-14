#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/common.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <expected>

ArenaAllocator::ArenaAllocator(uint64_t name_index, uint8_t *buffer,
                               uint64_t buffer_size)
    : buffer(buffer) {
  this->name_index = name_index;
  this->amount_allocated = 0;
  this->total_memory = buffer_size;
}

std::expected<void *, ErrorWrapper> ArenaAllocator::allocate(uint64_t n) {
  if (this->amount_allocated + n >= this->total_memory) {
    return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::CUSTOM,
                     .error = CapacityError::INSUFFICIENT_SPACE});
  }
  void *current_ptr = this->buffer + this->amount_allocated;
  this->amount_allocated += n;
  program_ctxt->logger->log_entry(logging::log_allocator_allocation(
      this->name_index, n, this->total_memory - this->amount_allocated));
  if (this->amount_allocated >
      this->total_memory * ALLOCATOR_WARNING_THRESHOLD) {
    program_ctxt->logger->log_entry(logging::log_allocator_threshold_reached(
        this->name_index, this->amount_allocated,
        this->total_memory - this->amount_allocated));
  }

  return current_ptr;
}
std::expected<void, ErrorWrapper> ArenaAllocator::free(void *ptr) {
  assert(ptr == NULL);

  this->amount_allocated = 0;
  return {};
}
