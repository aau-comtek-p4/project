#ifndef ARENA_ALLOCATOR_H
#define ARENA_ALLOCATOR_H
#include "general/interfaces/storage/allocator.h"
#include "general/misc/errors.h"
#include <cstdint>
class ArenaAllocator : public AllocatorInterface {
private:
  uint8_t *buffer;

public:
  ArenaAllocator(uint64_t name_index, uint8_t *buffer, uint64_t buffer_size);
  std::expected<void *, ErrorWrapper> allocate(uint64_t n) override;
  std::expected<void, ErrorWrapper> free(void *) override;
};
#endif
