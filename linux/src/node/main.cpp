
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

Task<int> sender() {
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (fcntl(client_socket, F_SETFL, O_NONBLOCK) < 0) {
    program_logger->log_err(NODE_ERROR_TAG,
                            "Failed to set socket to non blocking");
    safe_shutdown(ConfigurationError::FAILED_SETUP);
  }
  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(7000);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  auto res = co_await program_io->connect(client_socket, server_addr);
  if (!res.has_value()) {
    program_logger->log_err(NODE_ERROR_TAG, "Failed to connection to server");
    safe_shutdown(res.error().cust_error);
  }
  program_logger->log_info(NODE_TAG,
                           "Successfully established connection to server");
  const char *msg = "Hello world!!!";
  auto res2 =
      co_await program_io->send(client_socket, (uint8_t *)msg, strlen(msg));
  if (!res2.has_value()) {
    program_logger->log_err(NODE_ERROR_TAG, "Failed to send message to server");
    safe_shutdown(res2.error().cust_error);
  }
  program_logger->log_info(NODE_TAG, "Successfully sent message to server");
  auto res3 = program_buffer_allocator->allocate(1024);
  if (!res3.has_value()) {
    program_logger->log_err(NODE_ERROR_TAG,
                            "Failed to allocate buffer for server msg");
    safe_shutdown(res3.error());
  }
  uint8_t *buffer = (uint8_t *)res3.value();
  auto res4 = co_await program_io->recv(client_socket, buffer, 1024);
  if (!res4.has_value()) {
    program_logger->log_err(NODE_ERROR_TAG,
                            "Failed to receive message from server");
    safe_shutdown(res4.error().cust_error);
  }
  size_t bytes_read = res4.value();

  program_logger->log_info(NODE_TAG, "Read [%lu] from server", bytes_read);
  buffer[bytes_read] = 0;
  program_logger->log_info(NODE_TAG, "Received: [%s]", buffer);

  auto res5 = co_await program_io->close(client_socket);
  if (!res5.has_value()) {
    program_logger->log_err(NODE_ERROR_TAG,
                            "Failed to close connection to server");
    safe_shutdown(res5.error().cust_error);
  }

  program_logger->log_info(NODE_TAG, "Succesfully closed connection to server");
  auto res6 = program_buffer_allocator->free(buffer);
  if (!res6.has_value()) {
    program_logger->log_err(NODE_ERROR_TAG,
                            "Failed to free client buffer, error: [%s]",
                            custom_strerror(res6.error()));
    safe_shutdown(res6.error());
  }
  co_return 1;
}

Job keep_sending() {
  size_t a = 0;
  while (true) {
    co_await sender();
    a += 1;
    program_logger->log_info(NODE_TAG, "Sent: [%lu]", a);
    co_await sleep_for(ms_to_tick(1000));
  }
}

int main() {
  BasickClock clock(CLOCK_MONOTONIC, NS_PR_MS * CLOCK_MS_PR_TICK);
  program_clock = &clock;
  FPrintLogger logger;
  program_logger = &logger;
  const size_t arena_size = MAX_STACK_SIZE;

  uint8_t arena_buffer[arena_size] = {0};
  ArenaAllocator stack_allocator(arena_buffer, arena_size);

  program_logger->log_info(NODE_TAG, "Created arena, size: [%lu]", arena_size);
  init_globals(&stack_allocator);

  auto _ = spawn(keep_sending());

  _ = program_loop->run();
}
