#ifndef ESP_WIFI_UDP_TRANSPORT_H
#define ESP_WIFI_UDP_TRANSPORT_H

#include "general/interfaces/io/transports/udp_transport.h"

class ESPWIFIUDPTransport : public UDPIOTransport {
public:
  void cancel(const void *user_data) override;
  void submit() override;
  void process(uint64_t timeout) override;
  Task<std::expected<int, ErrorWrapper>>
  initialize(IOAddress *fd_addr, IOAddress *socket_addr) override;

  Task<std::expected<int, ErrorWrapper>> recv_from(IOAddress host_addr,
                                                   IOAddress *out_addr,
                                                   uint8_t *buf,
                                                   uint64_t buf_size) override;
  Task<std::expected<int, ErrorWrapper>> send_to(IOAddress host_addr,
                                                 IOAddress out_addr,
                                                 const uint8_t *buf,
                                                 uint64_t buf_size) override;

  Task<std::expected<int, ErrorWrapper>> io_close(IOAddress addr) override;
};

#endif
