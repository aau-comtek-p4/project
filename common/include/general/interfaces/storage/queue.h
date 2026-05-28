#ifndef QUEUE_INTERFACE_H

#define QUEUE_INTERFACE_H

#include "general/common.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <atomic>
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <mutex>
#include <utility>

#define QUEUE_TAG "QUEUE"
#define QUEUE_ERR_TAG "QUEUE ERR"
#define QUEUE_WARNING_THRESHOLD 0.8

template <typename T> class QueueInterface {
public:
  virtual uint64_t get_item_amount() = 0;
  virtual std::expected<void, ErrorWrapper> enque(T &&val) = 0;
  virtual std::expected<T, ErrorWrapper> deque() = 0;
  virtual std::expected<T, ErrorWrapper> peek() = 0;
};

template <typename T, uint64_t queue_size>
class Queue : public QueueInterface<T> {
private:
  alignas(64) std::atomic<uint64_t> head{0};
  alignas(64) std::atomic<uint64_t> tail{0};

  T storage[queue_size];

public:
  uint64_t get_item_amount() override {
    uint64_t h = head.load(std::memory_order_acquire);
    uint64_t t = tail.load(std::memory_order_acquire);

    if (t >= h) {
      return t - h;
    }

    return queue_size - h + t;
  }

  std::expected<void, ErrorWrapper> enque(T &&val) override {
    uint64_t t = tail.load(std::memory_order_relaxed);
    uint64_t next = (t + 1) % queue_size;

    uint64_t h = head.load(std::memory_order_acquire);

    // queue full
    if (next == h) {
      return std::unexpected(
          ErrorWrapper{.error = CapacityError::INSUFFICIENT_SPACE,
                       .tag = ErrorWrapper::CUSTOM});
    }

    storage[t] = std::move(val);

    tail.store(next, std::memory_order_release);

    return {};
  }

  std::expected<T, ErrorWrapper> deque() override {
    uint64_t h = head.load(std::memory_order_relaxed);

    uint64_t t = tail.load(std::memory_order_acquire);

    // queue empty
    if (h == t) {
      return std::unexpected(
          ErrorWrapper{.error = CapacityError::BUFFER_UNDERFLOW,
                       .tag = ErrorWrapper::CUSTOM});
    }

    T value = std::move(storage[h]);

    uint64_t next = (h + 1) % queue_size;

    head.store(next, std::memory_order_release);

    return value;
  }

  std::expected<T, ErrorWrapper> peek() override {
    uint64_t h = head.load(std::memory_order_relaxed);
    uint64_t t = tail.load(std::memory_order_acquire);

    if (h == t) {
      return std::unexpected(
          ErrorWrapper{.error = CapacityError::BUFFER_UNDERFLOW,
                       .tag = ErrorWrapper::CUSTOM});
    }

    return storage[h];
  }
};
#endif
