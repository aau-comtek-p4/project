
#include "common.h"
#include "common/context.h"
#include "common/coroutines/jobs/uart.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/context_innit.h"
#include "general/misc/names.h"
#include <asm-generic/ioctls.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <getopt.h>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <linux/serial.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
void handle_crlc(int sign) { program_ctxt->loop->stop(); }

int main(int argc, char *argv[]) {
  context_initialized = false;
  ProgramContext ctxt = {};
  std::this_thread::sleep_for(std::chrono::seconds(3));
  ContextConfig<ServerContextSettings> ctx_config;
  ctx_config.clock_type = CtxtClockType::WALL;
  ctx_config.ctx_type = ContextType::NODE;
  ctx_config.logger_type = CtxtLoggerType::FILE_LOGGER;
  ctx_config.log_serializer_type = CtxtLoggerSerializer::JSON_SERIALIZER;
  ctx_config.random_type = CtxtRandomType::NONE;
  ctx_config.deadline_tracker_type = CtxtDeadlineTrackerType::MIN_HEAP;
  uint8_t *total_buffer =
      (uint8_t *)calloc(sizeof(uint8_t), ctx_config.settings.max_total_size);
  ArenaAllocator total_allocator(NAME_PROGRAM_ALLOCATOR, total_buffer,
                                 ctx_config.settings.max_total_size);
  program_ctxt = &ctxt;

  innit_ctx(&ctxt, ctx_config, &total_allocator);
  program_ctxt->name_lookup->set_name(NAME_END, "print_job");
  program_ctxt->name_lookup->set_name(NAME_END + 1, "print_task");
  program_ctxt->name_lookup->set_name(NAME_END + 2, "cool_job");
  program_ctxt->name_lookup->set_name(NAME_END + 3, "write_job");
  program_ctxt->name_lookup->set_name(NAME_END + 4, "simple_print");
  program_ctxt->name_lookup->set_name(NAME_END + 5, "metric_logger");
  program_ctxt->name_lookup->set_name(NAME_END + 6, "limit_tester");
  program_ctxt->name_lookup->set_name(NAME_END + 7, "serial");
  signal(SIGINT, handle_crlc);
  program_ctxt->logger->submit();

  auto _ = spawn(uart_writer());
  _ = spawn(read_uart());

  _ = program_ctxt->loop->run();
  program_ctxt->logger->submit();
  program_ctxt->metrics->print_metrics();
  program_ctxt->logger->submit();
  free(total_buffer);

  return 0;
}
