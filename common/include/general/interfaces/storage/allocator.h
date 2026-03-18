#ifndef ALLOCATOR_INTERFACE_H
#define ALLOCATOR_INTERFACE_H

#include "general/common.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <format>

#define ALLOCATOR_TAG "ALLOCATOR"
#define ALLOCATOR_ERROR_TAG "ALLOCATOR ERROR"
class AllocatorInterface {
  virtual std::expected<void *, CapacityError> allocate(size_t n) = 0;
  virtual std::expected<void, CapacityError> free(void *) = 0;
};

#endif
