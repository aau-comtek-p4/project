#ifndef ARENA_ALLOCATOR_H
#define ARENA_ALLOCATOR_H
#include "general/interfaces/storage/allocator.h"
#include <cstdint>
class ArenaAllocator : public AllocatorInterface {
private:
  uint8_t *buffer;
  size_t buffer_size;

public:
  ArenaAllocator(uint8_t *buffer, size_t buffer_size);
  std::expected<void *, int> allocate(size_t n) override;
  std::expected<void, int> free(void *) override;
};
#endif
