#ifndef SIM_LOOP_H

#define SIM_LOOP_H

#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/storage/queue.h"
class SimLoop : EventLoopInterface {
private:
  QueueInterface<std::coroutine_handle<>> *ready_queue;
  QueueInterface<std::coroutine_handle<>> *staging_queue;
  AllocatorInterface *coroutine_generator_allocator;
  bool running;

public:
  SimLoop(QueueInterface<std::coroutine_handle<>> *ready_queue,
          QueueInterface<std::coroutine_handle<>> *staging_queue,
          AllocatorInterface *coroutine_generator_allocator);
  std::expected<void *, ErrorWrapper> allocate(size_t n) override;
  std::expected<void, ErrorWrapper> free(void *ptr) override;
  std::expected<void, ErrorWrapper>
  enque(std::coroutine_handle<> handle) override;
  std::expected<void, ErrorWrapper>
  enque_staging(std::coroutine_handle<> handle) override;

  std::expected<void, ErrorWrapper> set_future(std::coroutine_handle<> handle,
                                               uint64_t future_tick) override;
  std::expected<void, ErrorWrapper> step() override;
  std::expected<void, ErrorWrapper> run() override;
  void stop() override;
};

#endif
