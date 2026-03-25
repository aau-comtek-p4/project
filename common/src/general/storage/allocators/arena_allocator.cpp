#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/common.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>

ArenaAllocator::ArenaAllocator(uint8_t *buffer, size_t buffer_size)
    : buffer(buffer), buffer_size(buffer_size) {
  this->amount_allocated = 0;
}

std::expected<void *, ErrorWrapper> ArenaAllocator::allocate(size_t n) {
  if (this->amount_allocated + n >= this->buffer_size) {
    program_ctxt->logger->log_err(
        ALLOCATOR_ERROR_TAG,
        "Attempt to allocate arena allocator more than size");
    return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::CUSTOM,
                     .error = CapacityError::INSUFFICIENT_SPACE});
  }
  void *current_ptr = this->buffer + this->amount_allocated;
  this->amount_allocated += n;
  program_ctxt->logger->log_debug(ALLOCATOR_TAG, "Arena allocated: [%lu]", n);
  if (this->amount_allocated >
      this->buffer_size * ALLOCATOR_WARNING_THRESHOLD) {
    program_ctxt->logger->log_warning(
        ALLOCATOR_TAG,
        "Arena allocator usage exceeded warning threshold, threshold: [%f]",
        ALLOCATOR_WARNING_THRESHOLD);
  }

  return current_ptr;
}
std::expected<void, ErrorWrapper> ArenaAllocator::free(void *ptr) {
  assert(ptr == NULL);

  this->amount_allocated = 0;
  return {};
}
