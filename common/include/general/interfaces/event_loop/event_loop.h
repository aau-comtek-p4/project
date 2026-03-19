#ifndef EVENT_LOOP_INTERFACE_H
#define EVENT_LOOP_INTERFACE_H

#include "general/misc/errors.h"
#include <algorithm>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <expected>
#define EVENT_LOOP_TAG "EVENT LOOP"
#define EVENT_LOOP_ERR_TAG "EVENT LOOP ERR"
class EventLoopInterface {
public:
  virtual std::expected<void *, int> allocate(size_t n) = 0;
  virtual std::expected<void, int> enque(std::coroutine_handle<> handle) = 0;
  virtual std::expected<void, int>
  enque_staging(std::coroutine_handle<> handle) = 0;

  virtual std::expected<void, int> step() = 0;
  virtual std::expected<void, int> run() = 0;
};

#endif
