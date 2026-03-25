#ifndef ALLOCATOR_INTERFACE_H
#define ALLOCATOR_INTERFACE_H

#include "general/misc/errors.h"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <expected>

#define ALLOCATOR_TAG "ALLOCATOR"
#define ALLOCATOR_ERROR_TAG "ALLOCATOR ERROR"
#define ALLOCATOR_WARNING_THRESHOLD 0.8
class AllocatorInterface {
public:
  size_t amount_allocated;
  virtual std::expected<void *, ErrorWrapper> allocate(size_t n) = 0;
  virtual std::expected<void, ErrorWrapper> free(void *) = 0;
};

#endif
