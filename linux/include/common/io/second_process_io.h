#ifndef LINUX_SECOND_PROCESS_IO_H
#define LINUX_SECOND_PROCESS_IO_H
#include "general/common.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/tcp_transport.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/storage/allocators/bucket_allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/context_innit.h"
#include "general/misc/errors.h"
#include "general/misc/names.h"
#include "general/misc/shutdown.h"
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <functional>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define SHARED_MEM_BASE 0x600000000000

struct second_process_io_ctxt_t {
  int log_fd = 0;
  int child_pid;
  int efd_start;
  int efd_shutdown;
  uint8_t *shared_mem;
  ProgramContext *program_ctxt;
  ErrorWrapper *error;
};

inline second_process_io_ctxt_t second_process_ctxt = {0};

void initialize_second_process_io(uint64_t mem_size);
void second_process_io_loop();
void second_process_notify(int efd_queue);
void second_process_await_notify(int efd_queue);

void second_process_io_signal_end(ErrorWrapper error_wrapper);

class SecondProcessIOFileLogger : public LoggerInterface {
private:
  uint64_t dropped_count = 0;

public:
  SecondProcessIOFileLogger(QueueInterface<LogEntry> *msg_queue,
                            LogSerializerInterface *serializer);
  void log_entry(LogEntry entry) noexcept override;

  void submit(uint64_t timeout) noexcept override;
  void submit() noexcept override;
};

#endif
