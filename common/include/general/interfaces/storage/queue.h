#ifndef QUEUE_INTERFACE_H
#define QUEUE_INTERFACE_H

#include "general/misc/errors.h"
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <expected>
#include <utility>
template <typename T> class QueueInterface {
public:
  virtual std::expected<void, CapacityError> enque(T &&val) = 0;
  virtual std::expected<T *, CapacityError> deque() = 0;
};

template <typename T, size_t queue_size>
class Queue : public QueueInterface<T> {
private:
  T storage[queue_size];
  size_t head = 0;
  size_t tail = 0;
  size_t queue_items = 0;

public:
  std::expected<void, CapacityError> enque(T &&val) override {
    if (queue_items >= queue_size) {
      return std::unexpected(CapacityError::INSUFFICIENT_SPACE);
    }
    storage[tail] = std::move(val);
    tail = (tail + 1) % queue_size;
    queue_items += 1;
  };
  std::expected<T *, CapacityError> deque() override {
    if (queue_items <= 0) {
      return std::unexpected(CapacityError::BUFFER_UNDERFLOW);
    }
    T *ptr = &storage[head];
    head = (head + 1) % queue_size;
    queue_items -= 1;
    return ptr;
  }
};

template <size_t queue_size>
class CoRoutineQueue : public QueueInterface<std::coroutine_handle<>> {
private:
  std::coroutine_handle<> storage[queue_size];
  size_t head = 0;
  size_t tail = 0;
  size_t queue_items = 0;

public:
  std::expected<void, CapacityError> enque(std::coroutine_handle<> &&val) {
    if (queue_items >= queue_size) {
      return std::unexpected(CapacityError::INSUFFICIENT_SPACE);
    }
    storage[tail] = std::move(val);
    tail = (tail + 1) % queue_size;
    queue_items += 1;
  };
  std::expected<std::coroutine_handle<> *, CapacityError> deque() {
    if (queue_items <= 0) {
      return std::unexpected(CapacityError::BUFFER_UNDERFLOW);
    }
    std::coroutine_handle<> *ptr = &storage[head];
    head = (head + 1) % queue_size;
    queue_items -= 1;
    return ptr;
  }
};

#endif
