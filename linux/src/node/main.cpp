#include "common/context.h"
#include "common/io/io.h"
#include "common/io/transports/storage/blocking_file_write.h"
#include "common/logger/file_logger.h"
#include "common/node/common.h"

#include "common/node/context.h"
#include "common/server/context.h"
#include "general/awaiters/sleep_for.h"
#include "general/awaiters/timeout_awaiter.h"
#include "general/awaiters/yield_awaiter.h"
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/interfaces/simulator/random.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/storage/allocators/bucket_allocator.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/trace.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/names.h"
#include "general/misc/shutdown.h"
#include <coroutine>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <unistd.h>
bool ctcl_c = false;
void handle_sigint(int a) { ctcl_c = true; }
Job detect_ctrl_c() {
  CoRoutineCtxt *self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_CTRLC_ROUTINE);
  self_ctxt->trace.start();
  while (true) {
    if (ctcl_c) {
      program_ctxt->loop->stop();
      co_return;
    }
    co_await yield_coroutine();
  }
}
Task<int> delayed_printing() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_END + 1);
  self_ctxt->trace.start();
  co_await sleep_for(program_ctxt->clock->ms_to_tick(2000));
  if (self_ctxt->cancelled) {
    program_ctxt->logger->log_entry(
        logging::log_debug("print delayed cancelled"));
    co_return 0;
  }
  program_ctxt->logger->log_entry(logging::log_debug("print delayed"));
  co_return 0;
}
Job keep_printing() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_END);
  self_ctxt->trace.start();
  for (int i = 0; i < 5; i++) {
    program_ctxt->logger->log_entry(logging::log_debug("hello"));
    co_await delayed_printing();
  }
}
Job cool_job() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_END + 2);
  self_ctxt->trace.start();
  for (int i = 0; i < 5; i++) {
    co_await run_with_timeout(delayed_printing(),
                              program_ctxt->clock->ms_to_tick(1000));
  }
}
Job write_job() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_END + 3);
  self_ctxt->trace.start();
  auto transport =
      program_ctxt->io->get_transport<StorageIOTransport>(IOMethod::IO_FILE);
  const char *file_p = "text.txt";
  IOAddress file_path{.addr_type = IOAddress::FILE_PATH};
  strncpy(file_path.file_path, file_p, strlen(file_p));
  int fd = (co_await transport->io_open(file_path)).value();
  IOAddress file_d{.addr_type = IOAddress::FILE_DESCRIPTOR, .fd = fd};
  const char *out_txt = "eyoooooo";
  int bytes_written = (co_await transport->io_write(
                           file_d, (const uint8_t *)out_txt, strlen(out_txt)))
                          .value();
  int _ = (co_await transport->io_close(file_d)).value();
}
int main() {
  std::signal(SIGINT, handle_sigint);
  ContextConfig<NodeContextSettings> ctx_config;
  ctx_config.clock_type = CtxtClockType::WALL;
  ctx_config.ctx_type = ContextType::NODE;
  ctx_config.logger_type = CtxtLoggerType::FILE_LOGGER;
  ctx_config.log_serializer_type = CtxtLoggerSerializer::JSON_SERIALIZER;
  ctx_config.random_type = CtxtRandomType::NONE;
  ctx_config.deadline_tracker_type = CtxtDeadlineTrackerType::MIN_HEAP;
  ProgramContext ctxt;
  uint8_t total_buffer[ctx_config.settings.max_total_size];
  ArenaAllocator total_allocator(NAME_PROGRAM_ALLOCATOR, total_buffer,
                                 ctx_config.settings.max_total_size);

  innit_ctx(&ctxt, ctx_config, &total_allocator, NODE_TAG);
  program_ctxt->name_lookup->set_name(NAME_END, "print_job");
  program_ctxt->name_lookup->set_name(NAME_END + 1, "print_task");
  program_ctxt->name_lookup->set_name(NAME_END + 2, "cool_job");
  program_ctxt->name_lookup->set_name(NAME_END + 3, "write_job");

  auto _ = spawn(detect_ctrl_c());

  _ = spawn(keep_printing());
  _ = spawn(cool_job());
  _ = spawn(write_job());

  program_ctxt->clock->setup();
  _ = program_ctxt->loop->run(program_ctxt->clock->ms_to_tick(20000));

  safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
}
