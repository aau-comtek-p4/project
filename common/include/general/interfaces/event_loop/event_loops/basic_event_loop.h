#ifndef BASIC_EVENT_LOOP_H
#define BASIC_EVENT_LOOP_H

#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/storage/queue.h"
#include <coroutine>

class BasicEventLoop : public EventLoopInterface {
private:
  QueueInterface<std::coroutine_handle<>> *ready_queue;
  QueueInterface<std::coroutine_handle<>> *staging_queue;
  AllocatorInterface *coroutine_generator_allocator;

public:
  BasicEventLoop(QueueInterface<std::coroutine_handle<>> *ready_queue,
                 QueueInterface<std::coroutine_handle<>> *staging_queue,
                 AllocatorInterface *coroutine_generator_allocator);
  std::expected<void *, int> allocate(size_t n) override;
  std::expected<void, int> free(void *ptr) override;
  std::expected<void, int> enque(std::coroutine_handle<> handle) override;
  std::expected<void, int>
  enque_staging(std::coroutine_handle<> handle) override;

  std::expected<void, int> set_future(std::coroutine_handle<> handle,
                                      uint64_t future_tick) override;
  std::expected<void, int> step() override;
  std::expected<void, int> run() override;
};

#endif
