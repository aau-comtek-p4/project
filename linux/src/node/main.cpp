#include "common.h"
#include "common/context.h"
#include "common/io/io.h"
#include "general/awaiters/sleep_for.h"
#include "general/awaiters/yield_awaiter.h"
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/trace.h"
#include "general/misc/context.h"
#include "general/misc/context_innit.h"
#include "general/misc/errors.h"
#include "general/misc/names.h"
#include "general/misc/shutdown.h"
#include <arpa/inet.h>
#include <concepts>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <liburing.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
void handle_sigint(int a) { program_ctxt->loop->stop(); }
Job metric_logger() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_END);
  self_ctxt->trace.start();
  while (true) {
    co_await sleep_for(5000);
    program_ctxt->metrics->print_metrics();
  }
}
Job udp_handler() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_END + 1);
  self_ctxt->trace.start();
  IOAddress port_addr = {};
  port_addr.sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  port_addr.sockaddr.sin_port = htons(8080);
  port_addr.sockaddr.sin_family = AF_INET;
  port_addr.addr_type = IOAddress::IO_SOCKADDR;
  IOAddress fd_addr = {.addr_type = IOAddress::FILE_DESCRIPTOR};
  auto transport =
      program_ctxt->io->get_transport<UDPIOTransport>(IOMethod::IO_WIFI_UDP);
  transport->initialize(&fd_addr, &port_addr);
  IOAddress recv_addr = {};
  recv_addr.addr_type = IOAddress::IO_SOCKADDR;
  auto buf = (uint8_t *)program_ctxt->buffer_allocator->allocate(1024).value();
  char addr_buf[INET6_ADDRSTRLEN] = {0};
  while (true) {
    auto res = co_await transport->recv_from(fd_addr, &recv_addr, buf, 1024);
    printf("Got connection\n");
    if (!res.has_value()) {
      program_ctxt->logger->log_entry(logging::log_debug("Recv failed"));
      safe_shutdown(res.error());
    }
    inet_ntop(AF_INET, &recv_addr.sockaddr.sin_addr, addr_buf, INET_ADDRSTRLEN);
    printf("From: %s\n", addr_buf);
    printf("Got: ");
    for (int i = 0; i < res.value(); i++) {
      printf("%x ", buf[i]);
    }
    printf("\n");
  }
}
template <std::derived_from<ContextSettings> Config>
void test(ContextConfig<Config> config) {}
int main() {
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

  innit_ctx(&ctxt, ctx_config, &total_allocator);
  program_ctxt->name_lookup->set_name(NAME_END, "print_job");
  program_ctxt->name_lookup->set_name(NAME_END + 1, "udp_handler");
  program_ctxt->logger->submit();

  std::signal(SIGINT, handle_sigint);

  auto _ = spawn(metric_logger());
  _ = spawn(udp_handler());

  _ = program_ctxt->loop->run();

  safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
}
