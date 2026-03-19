#include "common/utility/clocks/basic_clock.h"
#include "common/utility/loggers/fprint_logger.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/storage/allocators/bucket_allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include <coroutine>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <liburing.h>
#include <unistd.h>

int main() {
  thread_local BasickClock basic_clock(CLOCK_MONOTONIC, NS_PR_MS * 10);
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
  auto res = spawn(keep_printing_boy());
  if (!res.has_value()) {
    tl_logger->log_err("MAIN", "Failed to spawn keep printing boy");
    return 1;
  }

  res = spawn(keep_printing_boy2());
  if (!res.has_value()) {
    tl_logger->log_err("MAIN", "Failed to spawn keep printing boy2");
    return 1;
  }
  res = loop.run();
}
