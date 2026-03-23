#ifndef LINUX_IO_H
#define LINUX_IO_H

#include "general/interfaces/io/io.h"
#include <cstddef>
#include <cstdint>
#include <liburing.h>

class LinuxIO : public IOInterface {
private:
  io_uring ring;
  size_t queue_depth;

public:
  LinuxIO(size_t queue_depth);
  Task<std::expected<int, int>> read(int id, uint8_t *out_buf,
                                     size_t max_read) override;
  Task<std::expected<int, int>> write(int id, uint8_t *in_buf,
                                      size_t write_amount) override;
  Task<std::expected<int, int>> open(const char *path, int flags,
                                     mode_t mode) override;
  Task<std::expected<int, int>> close(int fd) override;
  Task<std::expected<int, int>> accept(int id) override;
  Task<std::expected<int, int>> recv(int sock_fd, uint8_t *buf,
                                     size_t len) override;
  Task<std::expected<int, int>> send(int sock_fd, uint8_t *buf,
                                     size_t len) override;

  void submit() override;
  void process_cqe(uint64_t timeout_ns) override;
};

#endif
