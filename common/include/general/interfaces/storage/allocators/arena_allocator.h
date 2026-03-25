#ifndef ARENA_ALLOCATOR_H
#define ARENA_ALLOCATOR_H
#include "general/interfaces/storage/allocator.h"
#include "general/misc/errors.h"
#include <cstdint>
class ArenaAllocator : public AllocatorInterface {
private:
  uint8_t *buffer;
  size_t buffer_size;

public:
  ArenaAllocator(uint8_t *buffer, size_t buffer_size);
  std::expected<void *, ErrorWrapper> allocate(size_t n) override;
  std::expected<void, ErrorWrapper> free(void *) override;
};
#endif
