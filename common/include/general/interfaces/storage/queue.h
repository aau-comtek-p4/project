#ifndef QUEUE_INTERFACE_H
#define QUEUE_INTERFACE_H

#include "general/common.h"
#include "general/misc/errors.h"
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <expected>
#include <utility>

#define QUEUE_TAG "QUEUE"
#define QUEUE_ERR_TAG "QUEUE ERR"

template <typename T> class QueueInterface {
public:
  virtual std::expected<void, ErrorWrapper> enque(T &&val) = 0;
  virtual std::expected<T *, ErrorWrapper> deque() = 0;
};

template <typename T, size_t queue_size>
class Queue : public QueueInterface<T> {
private:
  T storage[queue_size];
  size_t head = 0;
  size_t tail = 0;
  size_t queue_items = 0;

public:
  std::expected<void, ErrorWrapper> enque(T &&val) override {
    if (queue_items >= queue_size) {
      return std::unexpected(
          ErrorWrapper{.tag = ErrorWrapper::CUSTOM,
                       .error = CapacityError::INSUFFICIENT_SPACE});
    }
    storage[tail] = std::move(val);
    tail = (tail + 1) % queue_size;
    queue_items += 1;
    return {};
  };
  std::expected<T *, ErrorWrapper> deque() override {
    if (queue_items <= 0) {
      return std::unexpected(
          ErrorWrapper{.tag = ErrorWrapper::CUSTOM,
                       .error = CapacityError::BUFFER_UNDERFLOW});
    }
    T *ptr = &storage[head];
    head = (head + 1) % queue_size;
    queue_items -= 1;
    return ptr;
  }
};
#endif
