#ifndef BASIC_EVENT_LOOP_H
#define BASIC_EVENT_LOOP_H

#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/storage/queue.h"
#include <coroutine>
#include <cstdint>

class BasicEventLoop : public EventLoopInterface {
private:
  QueueInterface<std::coroutine_handle<>> *general_queue;
  bool running;
  uint64_t max_ops;

public:
  BasicEventLoop(QueueInterface<std::coroutine_handle<>> *general_queue,
                 uint64_t max_ops);
  std::expected<void, ErrorWrapper>
  enque(std::coroutine_handle<> handle) override;
  std::expected<void, ErrorWrapper> enque_future(std::coroutine_handle<> handle,
                                                 uint64_t time_ms) override;

  std::expected<void, ErrorWrapper> run_step();
  std::expected<void, ErrorWrapper> step() override;
  std::expected<void, ErrorWrapper> run() override;
  std::expected<void, ErrorWrapper> run(uint64_t timeout) override;
  void stop() override;
};

#endif
