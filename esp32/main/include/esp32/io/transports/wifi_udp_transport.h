#ifndef ESP_WIFI_UDP_TRANSPORT_H
#define ESP_WIFI_UDP_TRANSPORT_H

#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/udp_transport.h"

class ESPWIFIUDPTransport : public UDPIOTransport {
public:
  Task<std::expected<int, ErrorWrapper>>
  initialize(const IOAddress *socket_addr) override;

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
  io_close(const IOAddress *addr) override;
};

#endif
