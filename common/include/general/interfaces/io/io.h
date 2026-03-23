#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H
#include "general/interfaces/event_loop/coroutines/task.h"
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
};

class IOInterface {
public:
  virtual Task<std::expected<int, int>> read(int id, uint8_t *out_buf,
                                             size_t max_read) = 0;
  virtual Task<std::expected<int, int>> write(int id, uint8_t *in_buf,
                                              size_t write_amount) = 0;
  virtual Task<std::expected<int, int>> open(const char *path, int flags,
                                             mode_t mode) = 0;
  virtual Task<std::expected<int, int>> close(int fd) = 0;

  virtual Task<std::expected<int, int>> accept(int id) = 0;
  virtual Task<std::expected<int, int>> recv(int sock_fd, uint8_t *buf,
                                             size_t len) = 0;

  virtual Task<std::expected<int, int>> send(int sock_fd, uint8_t *buf,
                                             size_t len) = 0;

  virtual void submit() = 0;
  virtual void process_cqe(uint64_t timeout_ns) = 0;
};

#endif
