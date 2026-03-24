#include "common.h"
#include "common/server/common.h"
#include "common/utility/clocks/basic_clock.h"
#include "common/utility/loggers/fprint_logger.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
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
#include <sys/socket.h>
#include <sys/types.h>
Job handle_client(int client_fd) {
  auto res = program_buffer_allocator->allocate(1024);
  if (!res.has_value()) {
    program_logger->log_err(SERVER_ERROR_TAG,
                            "Failed to allocate buffer for client on fd: [%u]",
                            client_fd);
    safe_shutdown(res.error());
  }

  program_logger->log_debug(SERVER_TAG, "Client handler for fd: [%u] started",
                            client_fd);
  uint8_t *buf = (uint8_t *)res.value();
  while (true) {
    auto res2 = co_await program_io->recv(client_fd, buf, 1024);
    if (!res2.has_value()) {
      program_logger->log_err(SERVER_ERROR_TAG,
                              "Failed to receive client data, error: [%u]",
                              res2.error().cust_error);
      safe_shutdown(res.error());
    }
    size_t bytes_read = res2.value();
    if (bytes_read == 0) {
      program_logger->log_info(SERVER_TAG, "Client on fd: [%u] diconnected",
                               client_fd);
      auto res = co_await program_io->close(client_fd);
      if (!res.has_value()) {
        program_logger->log_err(SERVER_ERROR_TAG,
                                "Failed to close client fd: [%u], errno: [%s]",
                                client_fd, strerror(res.error().error_number));
        safe_shutdown(res.error().cust_error);
      }

      program_logger->log_info(SERVER_TAG, "Closed client fd: [%u]", client_fd);
      auto buf_free_res = program_buffer_allocator->free(buf);
      if (!buf_free_res.has_value()) {
        program_logger->log_err(SERVER_ERROR_TAG,
                                "Failed to free client buffer, error: [%s]",
                                custom_strerror(res.error().cust_error));
        safe_shutdown(res.error().cust_error);
      }
      co_return;
    }
    program_logger->log_info(SERVER_TAG, "Read [%lu] bytes on fd:[%u]",
                             bytes_read, client_fd);
    buf[bytes_read] = 0;
    program_logger->log_info(SERVER_TAG, "Received: [%s]", buf);
    auto res3 = co_await program_io->send(client_fd, buf, bytes_read);
    if (!res3.has_value()) {
      program_logger->log_err(
          SERVER_ERROR_TAG,
          "Failed to send data to client fd: [%u], errno: [%s]", client_fd,
          strerror(res3.error().error_number));
      safe_shutdown(res3.error().cust_error);
    }
    program_logger->log_info(SERVER_TAG, "Echoed data back to client fd: [%u]",
                             client_fd);
  }

  co_return;
}

Job server() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0) {
    program_logger->log_err(SERVER_ERROR_TAG,
                            "Failed to set server socket to non blocking");
    safe_shutdown(ConfigurationError::FAILED_SETUP);
  }
  sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(SERVER_PORT);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_fd, (struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    program_logger->log_err(
        SERVER_ERROR_TAG,
        "Failed to bind server socket to port: [%u], errno: [%s]", SERVER_PORT,
        strerror(errno));
    safe_shutdown(ConfigurationError::FAILED_SETUP);
  }
  if (listen(server_fd, SERVER_CONNECTION_QUEUE_SIZE) < 0) {
    program_logger->log_err(SERVER_ERROR_TAG, "Failed to listen to port: [%u]",
                            SERVER_PORT);
    safe_shutdown(ConfigurationError::FAILED_SETUP);
  }
  program_logger->log_info(SERVER_TAG, "Server listening on port: [%u]",
                           SERVER_PORT);

  while (true) {
    auto res = co_await program_io->accept(server_fd);
    if (!res.has_value()) {
      program_logger->log_info(SERVER_TAG, "Accept failed: [%s]",
                               custom_strerror(res.error().cust_error));
    }
    program_logger->log_info(SERVER_TAG, "Connection received on fd: [%u]",
                             res.value());
    auto res2 = spawn(handle_client(res.value()));
    if (!res2.has_value()) {
      program_logger->log_err(SERVER_ERROR_TAG,
                              "Failed to spawn client handler, error: [%u]",
                              res2.error());
      safe_shutdown(res2.error());
    }
  }
  co_return;
}

int main() {
  BasickClock clock(CLOCK_MONOTONIC, NS_PR_MS * CLOCK_MS_PR_TICK);
  program_clock = &clock;
  FPrintLogger logger;
  program_logger = &logger;
  const size_t arena_size = MAX_STACK_SIZE;

  uint8_t arena_buffer[arena_size] = {0};
  ArenaAllocator stack_allocator(arena_buffer, arena_size);

  program_logger->log_info(SERVER_TAG, "Created arena, size: [%lu]",
                           arena_size);
  init_globals(&stack_allocator);

  auto res = spawn(server());
  res = program_loop->run();
}
