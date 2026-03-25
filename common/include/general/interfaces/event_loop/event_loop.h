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
  virtual std::expected<void *, ErrorWrapper> allocate(size_t n) = 0;
  virtual std::expected<void, ErrorWrapper> free(void *ptr) = 0;
  virtual std::expected<void, ErrorWrapper>
  enque(std::coroutine_handle<> handle) = 0;
  virtual std::expected<void, ErrorWrapper>
  enque_staging(std::coroutine_handle<> handle) = 0;

  virtual std::expected<void, ErrorWrapper>
  set_future(std::coroutine_handle<> handle, uint64_t future_tick) = 0;

  virtual std::expected<void, ErrorWrapper> step() = 0;
  virtual std::expected<void, ErrorWrapper> run() = 0;
  virtual void stop() = 0;
};

#endif
