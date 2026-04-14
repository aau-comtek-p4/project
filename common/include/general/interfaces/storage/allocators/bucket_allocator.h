#ifndef BUCKET_ALLOCATOR_H
#define BUCKET_ALLOCATOR_H
#include "general/common.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>

template <uint64_t bucket_size> struct Bucket {
  std::byte buffer[bucket_size];
  size_t allocation_size;
  Bucket<bucket_size> *next_ptr;
};

template <uint64_t bucket_count, uint64_t bucket_size>
class BucketAllocator : public AllocatorInterface {
private:
  uint64_t allocated_buckets_count;
  Bucket<bucket_size> *free_bucket_header_ptr;
  Bucket<bucket_size> *buffer;

public:
  BucketAllocator(uint64_t name_index, Bucket<bucket_size> *buffer);
  std::expected<void *, ErrorWrapper> allocate(uint64_t n) override;
  std::expected<void, ErrorWrapper> free(void *) override;
};

template <uint64_t bucket_count, uint64_t bucket_size>
BucketAllocator<bucket_count, bucket_size>::BucketAllocator(
    uint64_t name_index, Bucket<bucket_size> *buffer) {
  this->name_index = name_index;
  this->buffer = buffer;
  this->total_memory = bucket_count * bucket_size;
  this->amount_allocated = 0;
  this->free_bucket_header_ptr = this->buffer;
  this->allocated_buckets_count = 0;
  for (uint8_t i = 0; i < bucket_count - 1; i++) {
    this->buffer[i].next_ptr = &this->buffer[i + 1];
  }
  this->buffer[bucket_count - 1].next_ptr = nullptr;
}

template <uint64_t bucket_count, uint64_t bucket_size>
std::expected<void *, ErrorWrapper>
BucketAllocator<bucket_count, bucket_size>::allocate(uint64_t n) {
  if (n > bucket_size) {
    safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM,
                               .error = CapacityError::BUFFER_OVERFLOW});
  }
  if (this->free_bucket_header_ptr == nullptr) {
    return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::CUSTOM,
                     .error = CapacityError::INSUFFICIENT_SPACE});
  }
  assert((this->amount_allocated + n) <= this->total_memory);
  this->allocated_buckets_count += 1;
  Bucket<bucket_size> *current_bucket_index = this->free_bucket_header_ptr;
  assert(current_bucket_index->allocation_size == 0);
  current_bucket_index->allocation_size = n;

  this->free_bucket_header_ptr = current_bucket_index->next_ptr;
  this->amount_allocated += n;

  program_ctxt->logger->log_entry(logging::log_bucket_allocator_allocation(
      this->name_index, n, this->total_memory - this->amount_allocated,
      bucket_size));

  if (this->allocated_buckets_count >
      bucket_count * ALLOCATOR_WARNING_THRESHOLD) {
    program_ctxt->logger->log_entry(logging::log_allocator_threshold_reached(
        this->name_index, this->amount_allocated,
        this->total_memory - this->amount_allocated));
  }

  return current_bucket_index;
}

template <uint64_t bucket_count, uint64_t bucket_size>
std::expected<void, ErrorWrapper>
BucketAllocator<bucket_count, bucket_size>::free(void *bucket_ptr) {

  if (this->allocated_buckets_count == 0) {
    return std::unexpected(ErrorWrapper{
        .tag = ErrorWrapper::CUSTOM, .error = CapacityError::BUFFER_UNDERFLOW});
  }
  auto start = reinterpret_cast<std::byte *>(this->buffer);
  auto end = start + bucket_count * sizeof(Bucket<bucket_size>);
  auto ptr = reinterpret_cast<std::byte *>(bucket_ptr);
  bool ptr_in_range = ptr >= start && ptr < end;

  if (!ptr_in_range) {
    return std::unexpected(ErrorWrapper{.tag = ErrorWrapper::CUSTOM,
                                        .error = CapacityError::OUTSIDE_RANGE});
  }

  auto cur_bucket_ptr = reinterpret_cast<Bucket<bucket_size> *>(bucket_ptr);
  uint64_t offset = (reinterpret_cast<std::byte *>(cur_bucket_ptr) -
                     reinterpret_cast<std::byte *>(this->buffer)) %
                    sizeof(Bucket<bucket_size>);
  if (offset != 0) {
    return std::unexpected(ErrorWrapper{.tag = ErrorWrapper::CUSTOM,
                                        .error = CapacityError::OUTSIDE_RANGE});
  }
  if (cur_bucket_ptr->allocation_size == 0) {
    return std::unexpected(ErrorWrapper{
        .tag = ErrorWrapper::CUSTOM, .error = CapacityError::BUFFER_UNDERFLOW});
  }
  uint64_t used_memory = cur_bucket_ptr->allocation_size;
  cur_bucket_ptr->next_ptr = this->free_bucket_header_ptr;
  this->free_bucket_header_ptr = cur_bucket_ptr;
  this->allocated_buckets_count -= 1;
  cur_bucket_ptr->allocation_size = 0;
  this->amount_allocated -= used_memory;
  program_ctxt->logger->log_entry(
      logging::log_allocator_free(this->name_index, used_memory,
                                  this->total_memory - this->amount_allocated));
  return {};
}

#endif
