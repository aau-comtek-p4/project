#ifndef ESP_WIFI_TCP_TRANSPORT_H
#define ESP_WIFI_TCP_TRANSPORT_H
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/tcp_transport.h"
#include <cstdint>

class ESPWIFITCPTransport : public TCPIOTransport {
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

#endif
