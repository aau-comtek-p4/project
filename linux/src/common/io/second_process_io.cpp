#include "common/io/second_process_io.h"
#include "common.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/polling_io.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/wait.h>
#include <type_traits>
#include <unistd.h>
void initialize_second_process_io(uint64_t mem_size) {
  int mfd = memfd_create("Main process", MFD_CLOEXEC);
  if (mfd < 0) {
    printf("MEMFD failed\n");
    exit(errno);
  }
  uint64_t total_size =
      mem_size + sizeof(ProgramContext) + sizeof(ErrorWrapper);
  if (ftruncate(mfd, total_size) < 0) {
    printf("Ftrunc failed\n");
    exit(errno);
  }
  auto shared_mem = (uint8_t *)mmap(nullptr, total_size, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, mfd, 0);
  if (shared_mem == MAP_FAILED) {
    printf("MMAP failed\n");
    exit(errno);
  }
  int efd_start = eventfd(0, 0);
  int efd_shutdown = eventfd(0, 0);
  if (efd_start < 0 || efd_shutdown < 0) {
    printf("Event fd failed\n");
    exit(errno);
  }

  pid_t child = fork();
  if (child < 0) {
    printf("Fork failed\n");
    exit(errno);
  }
  second_process_ctxt = second_process_io_ctxt_t{
      .child_pid = child,
      .efd_start = efd_start,
      .efd_shutdown = efd_shutdown,
      .shared_mem = shared_mem,
      .program_ctxt = (ProgramContext *)(shared_mem + mem_size),
      .error = (ErrorWrapper *)(shared_mem + mem_size + sizeof(ProgramContext)),
  };
}
void second_process_notify(int efd_queue) {
  uint64_t opt = 1;
  int r = write(efd_queue, &opt, sizeof(opt));
  if (r < 0) {
    printf("Notify failed\n");
    exit(1);
  }
}
void second_process_await_notify(int efd_queue) {
  uint64_t opt;
  int r = read(efd_queue, &opt, sizeof(opt));

  if (r < 0) {
    printf("Read failed\n");
    exit(1);
  }
}

void second_process_io_signal_end(ErrorWrapper error_wrapper) {
  memcpy(second_process_ctxt.error, &error_wrapper, sizeof(ErrorWrapper));
  second_process_notify(second_process_ctxt.efd_shutdown);
  if (!second_process_ctxt.child_pid) {
    printf("Child exit\n");
    exit(1);

    return;
  }

  int res;
  waitpid(second_process_ctxt.child_pid, &res, 0);
  printf("raw status = %d\n", res);
  if (WIFEXITED(res)) {
    printf("child normal exit = %d\n", WEXITSTATUS(res));
  }

  if (WIFSIGNALED(res)) {
    printf("child killed by signal = %d\n", WTERMSIG(res));
  }

  if (WIFSTOPPED(res)) {
    printf("child stopped by signal = %d\n", WSTOPSIG(res));
  }
}

void second_process_io_loop() {
  second_process_await_notify(second_process_ctxt.efd_start);
  program_ctxt = second_process_ctxt.program_ctxt;
  printf("Child started\n");
  int log_fd = open(LOG_FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (log_fd < 0) {
    printf("failed open log file err %s\n", strerror(errno));
    exit(1);
  }
  uint8_t out_buf[1024];
  int epfd = epoll_create1(0);
  epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = second_process_ctxt.efd_shutdown;
  epoll_ctl(epfd, EPOLL_CTL_ADD, second_process_ctxt.efd_shutdown, &ev);

  while (true) {
    int n = epoll_wait(epfd, &ev, 1, 1);
    if (n) {
      second_process_await_notify(second_process_ctxt.efd_shutdown);
      printf("Child got shutdown\n");
      break;
    }
    poll_connections(out_buf);
    poll_general(out_buf);
  }
  printf("second process exit\n");
  exit(0);
}
