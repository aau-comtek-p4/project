#ifndef ALLOCATOR_INTERFACE_H
#define ALLOCATOR_INTERFACE_H

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <expected>

#define ALLOCATOR_TAG "ALLOCATOR"
#define ALLOCATOR_ERROR_TAG "ALLOCATOR ERROR"
class AllocatorInterface {
public:
  virtual std::expected<void *, int> allocate(size_t n) = 0;
  virtual std::expected<void, int> free(void *) = 0;
};

#endif
