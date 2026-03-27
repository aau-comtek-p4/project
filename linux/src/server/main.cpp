#include "common/server/common.h"
#include "common/server/context.h"
#include "common/utility/context.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/simulator/random.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/misc/context.h"
#include "general/misc/shutdown.h"
#include <coroutine>
#include <csignal>

bool ctcl_c = false;
void handle_sigint(int a) { ctcl_c = true; }

struct NothingAwaiter {
  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    auto _ = program_ctxt->loop->enque_staging(h);
  }
  void await_resume() {}
};

Job detect_ctrl_c() {
  while (true) {
    if (ctcl_c) {
      program_ctxt->logger->log_info(SERVER_TAG, "CTRL C pressed");
      safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
    }
    co_await NothingAwaiter();
  }
}

int main() {
  std::signal(SIGINT, handle_sigint);
  ContextConfig<ServerContextSettings> ctx_config;
  ctx_config.clock_type = CtxtClockType::SIM_CLOCK;
  ctx_config.ctx_type = ContextType::SIMULATOR;
  ctx_config.io_type = CtxtIOType::LINUX;
  ctx_config.logger_type = CtxtLoggerType::STDERR_LOGGER;
  ctx_config.random_type = CtxtRandomType::SEEDED;
  ctx_config.deadline_tracker_type = CtxtDeadlineTrackerType::MIN_HEAP;
  ctx_config.random_intervals[RandomType::IO_LATENCY] =
      RandomInterval{.min = 2, .max = 5};
  ctx_config.random_intervals[RandomType::MISSED_TICK] =
      RandomInterval{.min = 1, .max = 3};
  ctx_config.random_intervals[RandomType::MISSED_TICK_CHANCE] =
      RandomInterval{.min = 0, .max = 500};
  ctx_config.random_intervals[RandomType::NETWORK_LATENCY] =
      RandomInterval{.min = 20, .max = 50};
  ctx_config.random_intervals[RandomType::PACKET_DROP] =
      RandomInterval{.min = 0, .max = 200};
  ProgramContext ctxt;
  uint8_t total_buffer[ctx_config.settings.max_total_size];
  ArenaAllocator total_allocator(total_buffer,
                                 ctx_config.settings.max_total_size);
  innit_ctx(&ctxt, ctx_config, &total_allocator, SERVER_TAG);

  auto _ = spawn(detect_ctrl_c());
  _ = program_ctxt->loop->run();
}
