#ifndef LINUX_IO_H
#define LINUX_IO_H

#include "general/interfaces/io/io.h"
#include "general/misc/errors.h"
#include <cstddef>
#include <cstdint>
#include <liburing.h>

struct IOAddress {
  enum { IO_SOCKADDR, FILE_DESCRIPTOR, FILE_PATH, PORT } addr_type;
  union {
    sockaddr_in sockaddr;
    int fd;
    char file_path[30];
    uint16_t port;
  };
};
class IOAwaiterInterface : public IOAwaitInterface {
public:
  int fd;
  int result;
  const char *get_type();
  virtual void submit(io_uring_sqe *sqe) = 0;
  bool await_ready();
  bool await_suspend(std::coroutine_handle<
                     Task<std::expected<int, ErrorWrapper>>::promise_type>
                         handle);
  int await_resume();
};
class LibUringIO : public IOTransport {
public:
  void cancel(const void *user_data) override;
  void submit() override;
  void process(uint64_t timeout) override;
};
inline io_uring program_uring;

#endif
