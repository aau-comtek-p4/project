#include "common/context.h"
#include "common/io/io.h"
#include "common/io/transports/storage/blocking_file_write.h"
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
#include "general/interfaces/utility/trace.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <coroutine>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
bool ctcl_c = false;
void handle_sigint(int a) { ctcl_c = true; }
Job detect_ctrl_c() {
  while (true) {
    if (ctcl_c) {
      program_ctxt->logger->log_info(NODE_TAG, "CTRL C pressed");
      program_ctxt->loop->stop();
      co_return;
    }
    co_await yield_coroutine();
  }
}
Task<int> lazy_task() {
  CoRoutineCtxt *self_ctxt = co_await get_ctxt();
  self_ctxt->trace->set_name("Lazy task");
  self_ctxt->trace->start();
  co_await sleep_for(program_ctxt->clock->ms_to_tick(2000));
  program_ctxt->logger->log_info(NODE_TAG, "Done lazing around");
  co_return 0;
}
Job test_job() {
  program_ctxt->logger->log_info(NODE_TAG, "I have started");
  CoRoutineCtxt *self_ctxt = co_await get_ctxt();
  self_ctxt->trace->start();
  self_ctxt->trace->set_name("Test job");

  program_ctxt->logger->log_info(NODE_TAG, "My id is: [%lu]", self_ctxt->id);

  program_ctxt->logger->log_info(NODE_TAG, "Starting timeout");
  auto res = co_await run_with_timeout(lazy_task(),
                                       program_ctxt->clock->ms_to_tick(3000));
  if (res.has_value()) {
    program_ctxt->logger->log_info(NODE_TAG, "Task finished sucessfully");
  }
  co_await sleep_for(program_ctxt->clock->ms_to_tick(1000));
  self_ctxt->trace->print();
  program_ctxt->trace_handler->clear_trace(self_ctxt->trace);
}
int main() {
  std::signal(SIGINT, handle_sigint);
  ContextConfig<NodeContextSettings> ctx_config;
  ctx_config.clock_type = CtxtClockType::WALL;
  ctx_config.ctx_type = ContextType::NODE;
  ctx_config.logger_type = CtxtLoggerType::FWRITE_LOGGER;
  ctx_config.random_type = CtxtRandomType::NONE;
  ctx_config.deadline_tracker_type = CtxtDeadlineTrackerType::MIN_HEAP;
  ProgramContext ctxt;
  uint8_t total_buffer[ctx_config.settings.max_total_size];
  ArenaAllocator total_allocator(total_buffer,
                                 ctx_config.settings.max_total_size);

  innit_ctx(&ctxt, ctx_config, &total_allocator, NODE_TAG);
  new (program_ctxt->io->register_transport(
      IOMethod::IO_FILE, sizeof(BlockingFileWriteIOTransport)))
      BlockingFileWriteIOTransport(5);

  auto _ = spawn(detect_ctrl_c());
  _ = spawn(test_job());
  _ = spawn_future(lazy_task(), program_ctxt->clock->ms_to_tick(2000));

  program_ctxt->clock->setup();
  _ = program_ctxt->loop->run(program_ctxt->clock->ms_to_tick(10000));

  safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
}
