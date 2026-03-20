#include "common/server/common.h"
#include "common/utility/clocks/basic_clock.h"
#include "common/utility/loggers/fprint_logger.h"
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/deadline_storage/min_heap_storage.h"
#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/storage/allocators/bucket_allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include <chrono>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <liburing.h>
#include <thread>
#include <unistd.h>
using namespace std::chrono_literals;
uint64_t ms_to_tick(uint64_t time_ms) {
  return time_ms * NS_PR_MS / (SERVER_NS_PR_TICK);
}

struct SkipAwaiter {
  uint64_t timeout;
  SkipAwaiter(uint64_t timeout) : timeout(timeout) {}
  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    auto _ = tl_deadline_keeper->add_deadline(h, timeout);
  }
  void await_resume() {};
};

Job print_interval(const char *text, uint64_t interval) {
  size_t i = 0;
  while (true) {
    tl_logger->log_info(COROUTINE_TAG, "%s: %lu", text, i);
    co_await SkipAwaiter(interval);
    i++;
  }
}
template <typename T> class TimeoutAwaiter {
  uint64_t timeout;
  T routine;
  TimeoutAwaiter(uint64_t timeout) : timeout(timeout) {}
  void await_ready() { return; }
  void await_suspend(std::coroutine_handle<> h) {
    auto _ = tl_loop->allocate(sizeof(T));
    // tl_deadline_keeper->add_deadline(h, this->timeout);
  }
};

Task<int> async_print(const char *text) {

  tl_logger->log_info(COROUTINE_TAG, "%s, %u", text, 1);
  co_await SkipAwaiter(2000);
  tl_logger->log_info(COROUTINE_TAG, "%s, %u", text, 2);
  co_return 1;
}

int main() {
  thread_local BasickClock basic_clock(CLOCK_MONOTONIC, SERVER_NS_PR_TICK);
  tl_clock = &basic_clock;
  thread_local FPrintLogger logger;
  tl_logger = &logger;

  Bucket<300> routine_frame_buffer[20] = {};
  thread_local BucketAllocator<20, 300> routine_frame_allocator(
      routine_frame_buffer);
  tl_coroutine_frame_allocator = &routine_frame_allocator;
  Queue<std::coroutine_handle<>, 20> loop_ready_queue;
  Queue<std::coroutine_handle<>, 20> loop_staging_queue;
  Bucket<30> loop_generator_buffer[20] = {};
  BucketAllocator<20, 30> loop_generator_allocator(loop_generator_buffer);
  thread_local BasicEventLoop loop(&loop_ready_queue, &loop_staging_queue,
                                   &loop_generator_allocator);
  tl_loop = &loop;
  Deadline deadline_buffer[20];
  DeadlineMinHeap deadline_keeper(deadline_buffer, 20);
  tl_deadline_keeper = &deadline_keeper;
  auto _ = spawn_future(async_print("Hello sir"), 1210);
  _ = spawn_future(async_print("Hello sir"), 1210);
  _ = spawn(print_interval("Chud", 50));

  auto res = loop.run();
}
