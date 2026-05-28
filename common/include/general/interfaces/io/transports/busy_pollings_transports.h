#ifndef COMMON_BUSY_POLLING_IO_H
#define COMMON_BUSY_POLLING_IO_H
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/interfaces/io/transports/tcp_transport.h"
#include "general/interfaces/io/transports/udp_transport.h"
#include <cstdint>
namespace busy_polling {
// TCP
void prep_tcp_close(int host_fd, const void *user_data);
void prep_tcp_send(int client_fd, const uint8_t *buf, uint64_t buf_size,
                   const void *user_data, IOPackageType package_type,
                   bool with_ack);
void prep_tcp_connect(int host_fd, const void *connect_addr,
                      const void *user_data);
void prep_tcp_accept(int host_fd, void *recv_addr, const void *user_data);
void prep_tcp_recv(int client_fd, uint8_t *buf, uint64_t buf_size,
                   const void *user_data);
void prep_tcp_open(const void *host_addr, uint64_t listen_size,
                   IOTCPType tcp_type, const void *user_data);
// UDP
void prep_udp_open(const void *host_addr, uint64_t listen_size,
                   const void *user_data);
void prep_udp_sendto(int client_fd, const uint8_t *buf, uint64_t buf_size,
                     const void *send_addr, const void *user_data,
                     IOPackageType package_type, bool with_ack);
void prep_udp_recvfrom(int client_fd, uint8_t *buf, uint64_t buf_size,
                       void *recv_addr, const void *user_data);
void prep_udp_close(int host_fd, const void *conn_addr, const void *user_data);
//  FILE
void prep_file_open(const char *file_path, int flag, uint32_t mode,
                    const void *user_data);
void prep_file_write(int file_fd, const uint8_t *buf, uint64_t buf_size,
                     const void *user_data);
void prep_file_read(int file_fd, uint8_t *buf, uint64_t buf_size,
                    const void *user_data);
void prep_file_close(int file_fd, const void *user_data);
// SERIAL
void prep_serial_open(const char *file_path, const void *user_data);
void prep_serial_write(int file_fd, const uint8_t *buf, uint64_t buf_size,
                       const void *user_data, IOPackageType package_type,
                       bool with_ack);
void prep_serial_read(int file_fd, uint8_t *buf, uint64_t buf_size,
                      const void *user_data);
void prep_serial_close(int file_fd, const void *user_data);
} // namespace busy_polling

class BusyPollingUDPTransport : public UDPIOTransport {
public:
  Task<std::expected<int, ErrorWrapper>>
  initialize(const IOAddress *socket_addr, uint64_t listen_backlog) override;

  Task<std::expected<int, ErrorWrapper>> recv_from(const IOAddress *host_addr,
                                                   IOAddress *out_addr,
                                                   uint8_t *buf,
                                                   uint64_t buf_size) override;
  Task<std::expected<int, ErrorWrapper>>
  send_to(const IOAddress *host_addr, const IOAddress *out_addr,
          const uint8_t *buf, uint64_t buf_size,
          IOPackageType package_type) override;

  Task<std::expected<int, ErrorWrapper>>
  send_to(const IOAddress *host_addr, const IOAddress *out_addr,
          const uint8_t *buf, uint64_t buf_size, IOPackageType package_type,
          bool with_ack) override;

  Task<std::expected<int, ErrorWrapper>>
  io_close(const IOAddress *addr, const IOAddress *connection_addr) override;
};
class BusyPollingTCPTransport : public TCPIOTransport {
public:
  Task<std::expected<int, ErrorWrapper>>
  initialize(const IOAddress *socket_addr, uint64_t listen_backlog_size,
             IOTCPType tcp_type) override;

  Task<std::expected<int, ErrorWrapper>> accept(const IOAddress *host_addr,
                                                IOAddress *out_addr) override;

  Task<std::expected<int, ErrorWrapper>>
  connect(const IOAddress *host_addr, const IOAddress *out_addr) override;

  Task<std::expected<int, ErrorWrapper>>
  send(const IOAddress *out_addr, const uint8_t *buf, uint64_t buf_size,
       IOPackageType package_type) override;

  Task<std::expected<int, ErrorWrapper>>
  send(const IOAddress *out_addr, const uint8_t *buf, uint64_t buf_size,
       IOPackageType package_type, bool with_ack) override;

  Task<std::expected<int, ErrorWrapper>>
  recv(const IOAddress *out_addr, uint8_t *buf, uint64_t buf_size) override;

  Task<std::expected<int, ErrorWrapper>>
  io_close(const IOAddress *addr) override;
};
class BusyPollingSerialTransport : public StorageIOTransport {
private:
public:
  Task<std::expected<int, ErrorWrapper>>
  io_open(const IOAddress *addr) override;
  Task<std::expected<int, ErrorWrapper>>
  io_read(const IOAddress *addr, uint8_t *buf, uint64_t buf_size) override;
  Task<std::expected<int, ErrorWrapper>>
  io_write(const IOAddress *addr, const uint8_t *buf, uint64_t buf_size,
           IOPackageType package_type) override;

  Task<std::expected<int, ErrorWrapper>>
  io_write(const IOAddress *addr, const uint8_t *buf, uint64_t buf_size,
           IOPackageType package_type, bool with_ack) override;

  Task<std::expected<int, ErrorWrapper>>
  io_close(const IOAddress *addr) override;
};

class BusyPollingFileTransport : public StorageIOTransport {
private:
public:
  Task<std::expected<int, ErrorWrapper>>
  io_open(const IOAddress *addr) override;
  Task<std::expected<int, ErrorWrapper>>
  io_read(const IOAddress *addr, uint8_t *buf, uint64_t buf_size) override;
  Task<std::expected<int, ErrorWrapper>>
  io_write(const IOAddress *addr, const uint8_t *buf, uint64_t buf_size,
           IOPackageType package_type) override;

  Task<std::expected<int, ErrorWrapper>>
  io_write(const IOAddress *addr, const uint8_t *buf, uint64_t buf_size,
           IOPackageType package_type, bool with_ack) override;

  Task<std::expected<int, ErrorWrapper>>
  io_close(const IOAddress *addr) override;
};

#endif
