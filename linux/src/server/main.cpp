#include "common/context.h"
#include "common/io/io.h"
#include "common/io/transports/storage/blocking_file_write.h"
#include "common/server/common.h"
#include "common/server/context.h"
#include "general/awaiters/yield_awaiter.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/interfaces/simulator/random.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/utility/clock.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <coroutine>
#include <csignal>
#include <cstdint>
#include <cstring>

Job be_printing() {
  while (true) {
    program_ctxt->logger->log_info(SERVER_TAG, "I be printing");
    co_await yield_coroutine();
  }
}

bool ctcl_c = false;
void handle_sigint(int a) { ctcl_c = true; }
Job detect_ctrl_c() {
  while (true) {
    if (ctcl_c) {
      program_ctxt->logger->log_info(SERVER_TAG, "CTRL C pressed");
      program_ctxt->loop->stop();
      co_return;
    }
    co_await yield_coroutine();
  }
}

Job write_file(const char *file_p, const char *buf, uint64_t buf_size) {
  auto io_transport =
      program_ctxt->io->get_transport<StorageIOTransport>(IOMethod::IO_FILE);
  IOAddress open_addr{.addr_type = IOAddress::FILE_PATH, .file_path = "\0"};
  strncpy(open_addr.file_path, file_p, strlen(file_p));
  open_addr.file_path[strlen(file_p)] = 0;
  auto res = co_await io_transport->io_open(open_addr);
  if (!res.has_value()) {
    program_ctxt->logger->log_err(SERVER_ERROR_TAG,
                                  "Failed to open file, err: [%s]",
                                  custom_strerror(res.error()));
    safe_shutdown(res.error());
  }
  program_ctxt->logger->log_info(SERVER_TAG, "Created file: [%i]", res.value());
  IOAddress fd_addr{.addr_type = IOAddress::FILE_DESCRIPTOR, .fd = res.value()};
  res = co_await io_transport->io_write(fd_addr, (uint8_t *)buf, buf_size);

  if (!res.has_value()) {
    program_ctxt->logger->log_err(SERVER_ERROR_TAG,
                                  "Failed to write to file, err: [%s]",
                                  custom_strerror(res.error()));
    safe_shutdown(res.error());
  }

  program_ctxt->logger->log_info(SERVER_TAG, "Wrote [%i] bytes to file",
                                 res.value());

  res = co_await io_transport->io_close(fd_addr);

  if (!res.has_value()) {
    program_ctxt->logger->log_err(SERVER_ERROR_TAG,
                                  "Failed to close file, err: [%s]",
                                  custom_strerror(res.error()));
    safe_shutdown(res.error());
  }
  program_ctxt->logger->log_info(SERVER_TAG, "Close file");
}

int main() {
  std::signal(SIGINT, handle_sigint);
  ContextConfig<ServerContextSettings> ctx_config;
  ctx_config.clock_type = CtxtClockType::WALL;
  ctx_config.ctx_type = ContextType::NODE;
  ctx_config.logger_type = CtxtLoggerType::FWRITE_LOGGER;
  ctx_config.random_type = CtxtRandomType::NONE;
  ctx_config.deadline_tracker_type = CtxtDeadlineTrackerType::MIN_HEAP;
  ProgramContext ctxt;
  uint8_t total_buffer[ctx_config.settings.max_total_size];
  ArenaAllocator total_allocator(total_buffer,
                                 ctx_config.settings.max_total_size);
  innit_ctx(&ctxt, ctx_config, &total_allocator, SERVER_TAG);
  new (program_ctxt->io->register_transport(
      IOMethod::IO_FILE, sizeof(BlockingFileWriteIOTransport)))
      BlockingFileWriteIOTransport(5);

  auto _ = spawn(detect_ctrl_c());
  _ = spawn(be_printing());
  const char *txt = "Hello mom\n";
  _ = spawn(write_file("text.txt", txt, strlen(txt)));
  _ = spawn(write_file("text.txt", txt, strlen(txt)));

  program_ctxt->clock->setup();
  _ = program_ctxt->loop->run(program_ctxt->clock->ms_to_tick(10000));

  safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
}
