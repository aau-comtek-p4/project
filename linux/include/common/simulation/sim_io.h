#ifndef SIM_IO_H
#define SIM_IO_H

#include "general/interfaces/io/io.h"
#include "general/interfaces/storage/queue.h"
#include <asm-generic/socket.h>
#include <cstdint>
#include <netinet/in.h>

#define SIM_MAX_IN_FLIGHT 100
#define SIM_MAX_CTXT 5
#define RECV_BUFFER_SIZE 1024

struct SimPacket {
  enum { MSG, CONNECT };
  uint8_t *data;
  uint64_t data_len;
  uint64_t src_id;
  uint64_t dest_id;
  uint64_t delivery_tick;
  uint64_t send_tick;
  const void *user_data;
};
struct NetworkInfo {
  bool accepting[SIM_MAX_CTXT];
  uint64_t id;
};
struct Address {
  enum { TCP, SIM };
  union {
    sockaddr_in sock_addr;
    int id;
  };
};
class IOCordinator;

class SimIO : public IOInterface {
private:
  uint64_t queue_depth;
  QueueInterface<SimPacket> *connection_queue;
  uint64_t connection_queue_size;
  Queue<uint8_t, RECV_BUFFER_SIZE> tcp_stream[SIM_MAX_CTXT];
  IOCordinator *cordinator;
  NetworkInfo network_info;

public:
  SimIO(uint64_t queue_depth, uint64_t connection_queue_size);
  void append_msg(SimPacket packet);
  void append_connection(SimPacket packet);
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
  connect(int sock_fd, IOAddress server_addr) override;

  void cancel(const void *user_data) override;

  void submit() override;
  void process_cqe(uint64_t timeout_ns) override;
};

class IOCordinator {
private:
  SimIO ios[SIM_MAX_CTXT];

public:
  int accept(int id);
  int send(NetworkInfo *io, int id, uint8_t *buf, size_t len);
  int connect(NetworkInfo *io, int id);
  int recv(NetworkInfo *io, int id, uint8_t *buf, size_t len);
  int cancel(const void *user_data);
};

#endif
