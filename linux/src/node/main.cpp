#include "common.h"
#include "common/context.h"
#include "general/awaiters/sleep_for.h"
#include "general/awaiters/yield_awaiter.h"
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/trace.h"
#include "general/misc/context.h"
#include "general/misc/context_innit.h"
#include "general/misc/errors.h"
#include "general/misc/names.h"
#include "general/misc/shutdown.h"
#include <concepts>
#include <csignal>
#include <cstdint>
#include <unistd.h>
bool ctcl_c = false;
void handle_sigint(int a) { ctcl_c = true; }
Job detect_ctrl_c() {
  CoRoutineCtxt *self_ctxt = co_await get_ctxt();
  self_ctxt->log_debug = false;
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

Job metric_logger() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_END + 5);
  self_ctxt->trace.start();
  while (true) {
    co_await sleep_for(20000);
    program_ctxt->metrics->print_metrics();
  }
}
template <std::derived_from<ContextSettings> Config>
void test(ContextConfig<Config> config) {}
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
  program_ctxt->name_lookup->set_name(NAME_END + 4, "simple_print");
  program_ctxt->name_lookup->set_name(NAME_END + 5, "metric_logger");
  program_ctxt->name_lookup->set_name(NAME_END + 6, "limit_tester");

  auto _ = spawn(detect_ctrl_c());
  _ = spawn(metric_logger());

  _ = program_ctxt->loop->run(1000);

  safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
}
