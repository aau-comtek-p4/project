#ifndef BUCKET_ALLOCATOR_H
#define BUCKET_ALLOCATOR_H
#include "general/common.h"
#include "general/interfaces/storage/allocator.h"
#include "general/misc/errors.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>

template <size_t bucket_size> struct Bucket {
  std::byte buffer[bucket_size];
  size_t allocation_size;
  Bucket<bucket_size> *next_ptr;
};

template <size_t bucket_count, size_t bucket_size>
class BucketAllocator : public AllocatorInterface {
private:
  size_t allocated_buckets_count;
  Bucket<bucket_size> *buffer;
  Bucket<bucket_size> *free_bucket_header_ptr;

public:
  size_t total_memory;
  size_t used_memory;
  BucketAllocator(Bucket<bucket_size> *buffer);
  std::expected<void *, int> allocate(size_t n) override;
  std::expected<void, int> free(void *) override;
  std::expected<void, int> print_bucket(void *);
};

template <size_t bucket_count, size_t bucket_size>
BucketAllocator<bucket_count, bucket_size>::BucketAllocator(
    Bucket<bucket_size> *buffer) {
  this->buffer = buffer;
  this->total_memory = bucket_count * bucket_size;
  this->used_memory = 0;
  this->free_bucket_header_ptr = this->buffer;
  this->allocated_buckets_count = 0;
  for (uint8_t i = 0; i < bucket_count - 1; i++) {
    this->buffer[i].next_ptr = &this->buffer[i + 1];
  }
  this->buffer[bucket_count - 1].next_ptr = nullptr;
}

template <size_t bucket_count, size_t bucket_size>
std::expected<void, int>
BucketAllocator<bucket_count, bucket_size>::print_bucket(void *bucket_ptr) {
  bool ptr_in_range =
      bucket_ptr >= this->buffer && bucket_ptr <= (this->buffer + bucket_count);
  if (!ptr_in_range) {
    tl_logger->log_err(ALLOCATOR_ERROR_TAG,
                       "Attempt to print bucket ptr outside buffer");
    return std::unexpected(CapacityError::OUTSIDE_RANGE);
  }

  auto cur_bucket_ptr = (Bucket<bucket_size> *)bucket_ptr;
  if ((cur_bucket_ptr - this->buffer) % sizeof(Bucket<bucket_size>) != 0) {
    tl_logger->log_err(ALLOCATOR_ERROR_TAG,
                       "Attempt to print bucket ptr not aligned with bucket");
    return std::unexpected(CapacityError::OUTSIDE_RANGE);
  }

  char out_str[sizeof(Bucket<bucket_size>) * 3 + 1];
  for (size_t index = 0; index < sizeof(Bucket<bucket_size>); index++) {
    snprintf(&out_str[index * 3], 4, "%02X,", ((uint8_t *)bucket_ptr)[index]);
  }

  out_str[sizeof(Bucket<bucket_size>) * 3 - 1] = '\0';
  tl_logger->log_info(ALLOCATOR_TAG, "%s", out_str);

  return {};
}

template <size_t bucket_count, size_t bucket_size>
std::expected<void *, int>
BucketAllocator<bucket_count, bucket_size>::allocate(size_t n) {
  assert(n <= bucket_size);
  if (this->free_bucket_header_ptr == nullptr) {
    tl_logger->log_err(ALLOCATOR_ERROR_TAG,
                       "Attempt to allocate when no more buckets");
    return std::unexpected(CapacityError::INSUFFICIENT_SPACE);
  }
  assert((this->used_memory + n) <= this->total_memory);
  this->allocated_buckets_count += 1;
  Bucket<bucket_size> *current_bucket_index = this->free_bucket_header_ptr;
  assert(current_bucket_index->allocation_size == 0);
  current_bucket_index->allocation_size = n;

  this->free_bucket_header_ptr = current_bucket_index->next_ptr;
  this->used_memory += n;
  tl_logger->log_debug(ALLOCATOR_TAG, "Allocated bucket, ussed bytes; [%lu]",
                       n);
  return current_bucket_index;
}

template <size_t bucket_count, size_t bucket_size>
std::expected<void, int>
BucketAllocator<bucket_count, bucket_size>::free(void *bucket_ptr) {

  if (this->allocated_buckets_count == 0) {
    tl_logger->log_err(ALLOCATOR_ERROR_TAG,
                       "Attempt to free when none allocated");
    return std::unexpected(CapacityError::BUFFER_UNDERFLOW);
  }
  auto start = reinterpret_cast<std::byte *>(this->buffer);
  auto end = start + bucket_count * sizeof(Bucket<bucket_size>);
  auto ptr = reinterpret_cast<std::byte *>(bucket_ptr);
  bool ptr_in_range = ptr >= start && ptr < end;

  if (!ptr_in_range) {
    tl_logger->log_err(ALLOCATOR_ERROR_TAG,
                       "Attempt to free address outside buffer");
    return std::unexpected(CapacityError::OUTSIDE_RANGE);
  }

  auto cur_bucket_ptr = reinterpret_cast<Bucket<bucket_size> *>(bucket_ptr);
  size_t offset = (reinterpret_cast<std::byte *>(cur_bucket_ptr) -
                   reinterpret_cast<std::byte *>(this->buffer)) %
                  sizeof(Bucket<bucket_size>);
  if (offset != 0) {
    tl_logger->log_err(
        ALLOCATOR_ERROR_TAG,
        "Attempt to free address not aligned with bucket, offset: [%lu]",
        offset);
    return std::unexpected(CapacityError::OUTSIDE_RANGE);
  }
  size_t used_memory = cur_bucket_ptr->allocation_size;
  assert((this->used_memory - used_memory) >= 0);
  cur_bucket_ptr->next_ptr = this->free_bucket_header_ptr;
  this->free_bucket_header_ptr = cur_bucket_ptr;
  this->allocated_buckets_count -= 1;
  cur_bucket_ptr->allocation_size = 0;
  this->used_memory -= used_memory;
  tl_logger->log_debug(ALLOCATOR_TAG, "Freed bucket, freed bytes: [%lu]",
                       used_memory);
  return {};
}

#endif
