#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/misc/errors.h"
#include "netinet/in.h"
#include <bits/types/struct_iovec.h>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <sys/types.h>

#define IO_TAG "IO"
#define IO_ERROR_TAG "IO ERROR"

enum IOType {
  READ,
  WRITE,
  OPEN,
  CLOSE,
  ACCEPT,
  RECV,
  SEND,
  CONNECT,
};
class IOAwaitInterface {
public:
  std::coroutine_handle<> handle;
  virtual void set_result(int result) = 0;
  virtual const char *get_type() = 0;
  virtual bool await_ready() = 0;
  virtual void
  await_suspend(std::coroutine_handle<
                Task<std::expected<int, ErrorWrapper>>::promise_type>
                    handle) = 0;
  virtual int await_resume() = 0;
};

class IOInterface {
public:
  virtual Task<std::expected<int, ErrorWrapper>> read(int id, uint8_t *out_buf,
                                                      size_t max_read) = 0;
  virtual Task<std::expected<int, ErrorWrapper>> write(int id, uint8_t *in_buf,
                                                       size_t write_amount) = 0;
  virtual Task<std::expected<int, ErrorWrapper>>
  open(const char *path, int flags, mode_t mode) = 0;
  virtual Task<std::expected<int, ErrorWrapper>> close(int fd) = 0;

  virtual Task<std::expected<int, ErrorWrapper>> accept(int id) = 0;
  virtual Task<std::expected<int, ErrorWrapper>> recv(int sock_fd, uint8_t *buf,
                                                      size_t len) = 0;

  virtual Task<std::expected<int, ErrorWrapper>> send(int sock_fd, uint8_t *buf,
                                                      size_t len) = 0;

  virtual Task<std::expected<int, ErrorWrapper>>
  connect(int sock_fd, sockaddr_in server_addr) = 0;

  virtual void cancel(const void *user_data) = 0;
  virtual void submit() = 0;
  virtual void process_cqe(uint64_t timeout_ns) = 0;
};

#endif
