
#include "common/server/common.h"
#include "common/utility/loggers/fprint_logger.h"

#include "common/utility/clocks/basic_clock.h"
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
#include <sys/uio.h>

Task<int> test() {
  auto res = co_await run_with_timeout(
      program_io->open("hello.txt", O_RDWR | O_CREAT, 0644), ms_to_tick(1000));
  if (!res->has_value()) {
    program_logger->log_err("MAIN", "Failed to open file");
    co_return OpenError::OPEN_FAILED;
  }
  int fd = res->value();
  program_logger->log_debug("MAIN", "Opened fd, [%i]", fd);
  const char *text = "Hello son";
  size_t si = strlen(text);

  res = co_await run_with_timeout(program_io->write(fd, (uint8_t *)text, si),
                                  ms_to_tick(1000));

  if (!res->has_value()) {
    program_logger->log_err("MAIN", "Failed to write to file");
    co_return WriteError::WRITE_FAILED;
  }

  size_t bytes_written = res->value();
  program_logger->log_debug("MAIN", "Wrote [%lu] bytes to file", bytes_written);
  char text_out[21];
  auto res2 = co_await run_with_timeout(
      program_io->read(fd, (uint8_t *)text_out, bytes_written),
      ms_to_tick(1000));
  if (!res2->has_value()) {
    program_logger->log_err("MAIN", "Failed to read to file");
    co_return ReadError::READ_FAILED;
  }
  text_out[bytes_written] = 0;
  program_logger->log_debug("MAIN", "Read [%i] bytes, content: [%s]",
                            res2->value(), text_out);
  co_return 0;
};

Task<int> run_shutdown() {
  auto res = co_await test();
  if (res) {
    safe_shutdown(res);
  }
  co_return 1;
}
Task<int> alul() {
  program_logger->log_debug("MAIN", "HELLO");
  co_return 1;
}

int main() {

  const uint64_t clock_tick_ms = 10;
  BasickClock clock(CLOCK_MONOTONIC, NS_PR_MS * clock_tick_ms);
  program_clock = &clock;
  FPrintLogger logger;
  program_logger = &logger;
  const size_t arena_size = 1024 * 40;

  uint8_t arena_buffer[arena_size] = {0};
  ArenaAllocator stack_allocator(arena_buffer, arena_size);

  program_logger->log_info("MAIN", "Created arena, size: [%lu]", arena_size);
  init_globals(&stack_allocator);
  program_logger->log_info("MAIN", "Globals total allocated: [%lu]",
                           stack_allocator.amount_allocated);

  auto _ = spawn(run_shutdown());

  _ = spawn(run_shutdown());

  _ = program_loop->run();
}
