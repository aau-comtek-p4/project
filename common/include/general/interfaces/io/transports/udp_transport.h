#ifndef UDP_TRANSPORT_H
#define UDP_TRANSPORT_H
#include "general/interfaces/io/io.h"
#include <cstdint>

class UDPIOTransport : public IOTransport {
public:
  virtual Task<std::expected<int, ErrorWrapper>>
  initialize(const IOAddress *socket_addr, uint64_t listen_backlog) = 0;

  virtual Task<std::expected<int, ErrorWrapper>>
  recv_from(const IOAddress *host_addr, IOAddress *out_addr, uint8_t *buf,
            uint64_t buf_size) = 0;
  virtual Task<std::expected<int, ErrorWrapper>>
  send_to(const IOAddress *host_addr, const IOAddress *out_addr,
          const uint8_t *buf, uint64_t buf_size,
          IOPackageType package_type) = 0;
  virtual Task<std::expected<int, ErrorWrapper>>
  send_to(const IOAddress *host_addr, const IOAddress *out_addr,
          const uint8_t *buf, uint64_t buf_size, IOPackageType package_type,
          bool with_ack) = 0;

  virtual Task<std::expected<int, ErrorWrapper>>
  io_close(const IOAddress *addr, const IOAddress *conneciton_addr) = 0;
};

#endif
