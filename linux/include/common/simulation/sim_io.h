#ifndef SIM_IO_H
#define SIM_IO_H

#include "general/interfaces/io/io.h"
class SimIO : public IOInterface {
private:
  size_t queue_depth;

public:
  SimIO(size_t queue_depth);
  Task<std::expected<int, ErrorWrapper>> read(int id, uint8_t *out_buf,
                                              size_t max_read) override;
  Task<std::expected<int, ErrorWrapper>> write(int id, uint8_t *in_buf,
                                               size_t write_amount) override;
  Task<std::expected<int, ErrorWrapper>> open(const char *path, int flags,
                                              mode_t mode) override;
  Task<std::expected<int, ErrorWrapper>> close(int fd) override;
  Task<std::expected<int, ErrorWrapper>> accept(int id) override;
  Task<std::expected<int, ErrorWrapper>> recv(int sock_fd, uint8_t *buf,
                                              size_t len) override;
  Task<std::expected<int, ErrorWrapper>> send(int sock_fd, uint8_t *buf,
                                              size_t len) override;
  Task<std::expected<int, ErrorWrapper>>
  connect(int sock_fd, sockaddr_in server_addr) override;

  void cancel(const void *user_data) override;

  void submit() override;
  void process_cqe(uint64_t timeout_ns) override;
};

#endif
