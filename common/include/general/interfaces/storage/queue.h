#ifndef QUEUE_INTERFACE_H

#define QUEUE_INTERFACE_H

#include "general/common.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <utility>

#define QUEUE_TAG "QUEUE"
#define QUEUE_ERR_TAG "QUEUE ERR"
#define QUEUE_WARNING_THRESHOLD 0.8

template <typename T> class QueueInterface {
public:
  uint64_t name_index;
  virtual std::expected<void, ErrorWrapper> enque(T &&val) = 0;
  virtual std::expected<T, ErrorWrapper> deque() = 0;
  virtual std::expected<T, ErrorWrapper> peek() = 0;
};

template <typename T, size_t queue_size>
class Queue : public QueueInterface<T> {
private:
  T storage[queue_size];
  uint64_t head = 0;
  uint64_t tail = 0;
  uint64_t queue_items = 0;

public:
  Queue(uint64_t name_index) { this->name_index = name_index; };
  std::expected<void, ErrorWrapper> enque(T &&val) override {
    if (queue_items >= queue_size) {
      return std::unexpected(
          ErrorWrapper{.error = CapacityError::INSUFFICIENT_SPACE,
                       .tag = ErrorWrapper::CUSTOM});
    }
    storage[tail] = std::move(val);
    tail = (tail + 1) % queue_size;
    queue_items += 1;
    return {};
  };
  std::expected<T, ErrorWrapper> deque() override {
    if (queue_items <= 0) {
      return std::unexpected(
          ErrorWrapper{.error = CapacityError::BUFFER_UNDERFLOW,
                       .tag = ErrorWrapper::CUSTOM});
    }
    T ptr = storage[head];
    head = (head + 1) % queue_size;
    queue_items -= 1;
    return ptr;
  }
  std::expected<T, ErrorWrapper> peek() override {
    if (queue_items <= 0) {
      return std::unexpected(
          ErrorWrapper{.error = CapacityError::BUFFER_UNDERFLOW,
                       .tag = ErrorWrapper::CUSTOM});
    }
    T ptr = storage[head];
    return ptr;
  }
};
#endif
