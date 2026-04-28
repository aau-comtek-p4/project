#ifndef ALLOCATOR_INTERFACE_H
#define ALLOCATOR_INTERFACE_H

#include "general/misc/errors.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>

#define ALLOCATOR_TAG "ALLOCATOR"
#define ALLOCATOR_ERROR_TAG "ALLOCATOR ERROR"
#define ALLOCATOR_WARNING_THRESHOLD 0.8
#define ALLOCATOR_LOGGING 0
class AllocatorInterface {
public:
  uint64_t amount_allocated;
  uint64_t total_memory;
  uint64_t name_index;
  virtual std::expected<void *, ErrorWrapper> allocate(uint64_t n) = 0;
  virtual std::expected<void, ErrorWrapper> free(void *) = 0;
};

#endif
