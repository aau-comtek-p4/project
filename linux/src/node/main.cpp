
#include "common.h"
#include "common/node/common.h"
#include "common/utility/loggers/fprint_logger.h"

#include "common/utility/clocks/basic_clock.h"
#include "general/awaiters/sleep_for.h"
#include "general/awaiters/timeout_awaiter.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

int main() {
  ProgramContext ctxt;
  program_ctxt = &ctxt;
  BasickClock clock(CLOCK_MONOTONIC, NS_PR_MS * CLOCK_MS_PR_TICK);
  program_ctxt->clock = &clock;
  FPrintLogger logger;
  program_ctxt->logger = &logger;
  const size_t arena_size = MAX_STACK_SIZE;

  uint8_t arena_buffer[arena_size] = {0};
  ArenaAllocator stack_allocator(arena_buffer, arena_size);

  program_ctxt->logger->log_info(NODE_TAG, "Created arena, size: [%lu]",
                                 arena_size);
  node_init_ctxt(program_ctxt, &stack_allocator);

  uint64_t start_time = program_ctxt->clock->spin_untill_future();
  program_ctxt->clock->set_future_tick(start_time);
  auto _ = program_ctxt->loop->run();
  safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
}
