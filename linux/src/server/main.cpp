#include "common.h"
#include "common/server/common.h"
#include "common/simulation/sim_clock.h"
#include "common/utility/clocks/basic_clock.h"
#include "common/utility/loggers/fprint_logger.h"
#include "general/awaiters/sleep_for.h"
#include "general/awaiters/timeout_awaiter.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/utility/clock.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <liburing.h>
#include <linux/io_uring.h>
#include <netinet/in.h>
#include <new>
#include <sys/socket.h>
#include <sys/types.h>
enum ServerType {
  UPD_SERVER,
  TCP_SERVER,
};
int setup_server(ServerType server_type) {
  int socket_server_type;
  switch (server_type) {
  case ServerType::TCP_SERVER:
    socket_server_type = SOCK_STREAM;
    break;
  case ServerType::UPD_SERVER:
    socket_server_type = SOCK_DGRAM;
    break;
  }
  ErrorWrapper setup_error{.tag = ErrorWrapper::CUSTOM,
                           .error = ConfigurationError::FAILED_SETUP};
  int server_fd = socket(AF_INET, socket_server_type, 0);
  if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0) {
    program_ctxt->logger->log_err(
        SERVER_ERROR_TAG, "Failed to set server socket to non blocking");
    safe_shutdown(setup_error);
  }
  sockaddr_in server_addr;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    program_ctxt->logger->log_err(
        SERVER_ERROR_TAG,
        "Failed to bind server socket on port: [%u], error: [%s]", SERVER_PORT,
        custom_strerror(
            ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = errno}));
    safe_shutdown(setup_error);
  }
  if (listen(server_fd, SERVER_CONNECTION_QUEUE_SIZE) < 0) {
    program_ctxt->logger->log_err(
        SERVER_ERROR_TAG, "Server failed to listen on port: [%u], error: [%s]",
        SERVER_PORT,
        custom_strerror(
            ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = errno}));
    safe_shutdown(setup_error);
  }
  return server_fd;
}

Job server() {
  int socket_fd = setup_server(ServerType::TCP_SERVER);

  while (true) {
    program_ctxt->logger->log_info(SERVER_TAG, "Server accepting connections");
    auto res =
        co_await run_with_timeout(program_ctxt->io->accept(socket_fd), 11);
    if (!res.has_value()) {
      program_ctxt->logger->log_err(SERVER_ERROR_TAG,
                                    "Server accept error: [%s]",
                                    custom_strerror(res.error()));
      co_await sleep_for(2000);
    } else {
      program_ctxt->logger->log_info(SERVER_TAG, "Got connection on fd: [%u]",
                                     res.value());
    }
  }
}
Job shutdown() {
  safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
  co_return;
}
int main() {
  ProgramContext ctxt;
  program_ctxt = &ctxt;
  // BasickClock clock(CLOCK_MONOTONIC, NS_PR_MS * CLOCK_MS_PR_TICK);
  SimClock clock;
  program_ctxt->clock = &clock;
  FPrintLogger logger;
  program_ctxt->logger = &logger;
  const size_t arena_size = MAX_STACK_SIZE;

  uint8_t arena_buffer[arena_size] = {0};
  ArenaAllocator stack_allocator(arena_buffer, arena_size);

  program_ctxt->logger->log_info(SERVER_TAG, "Created arena, size: [%lu]",
                                 arena_size);
  server_init_ctxt(program_ctxt, &stack_allocator);

  auto res = spawn(server());
  res = spawn_future(shutdown(), 20000);
  uint64_t start_time = program_ctxt->clock->spin_untill_future();
  program_ctxt->clock->set_future_tick(start_time);
  res = program_ctxt->loop->run();
  safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
}
