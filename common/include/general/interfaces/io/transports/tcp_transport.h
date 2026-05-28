#ifndef TCP_TRANSPORT_H
#define TCP_TRANSPORT_H
#include "general/interfaces/io/io.h"
#include <cstdint>

class TCPIOTransport : public IOTransport {
public:
  virtual Task<std::expected<int, ErrorWrapper>>
  initialize(const IOAddress *socket_addr, uint64_t listen_backlog_size,
             IOTCPType tcp_type) = 0;

  virtual Task<std::expected<int, ErrorWrapper>>
  accept(const IOAddress *host_addr, IOAddress *out_addr) = 0;

  virtual Task<std::expected<int, ErrorWrapper>>
  connect(const IOAddress *host_addr, const IOAddress *out_addr) = 0;

  virtual Task<std::expected<int, ErrorWrapper>>
  send(const IOAddress *out_addr, const uint8_t *buf, uint64_t buf_size,
       IOPackageType package_type) = 0;
  virtual Task<std::expected<int, ErrorWrapper>>
  send(const IOAddress *out_addr, const uint8_t *buf, uint64_t buf_size,
       IOPackageType package_type, bool with_ack) = 0;
  virtual Task<std::expected<int, ErrorWrapper>>
  recv(const IOAddress *out_addr, uint8_t *buf, uint64_t buf_size) = 0;

  virtual Task<std::expected<int, ErrorWrapper>>
  io_close(const IOAddress *addr) = 0;
};

#endif
