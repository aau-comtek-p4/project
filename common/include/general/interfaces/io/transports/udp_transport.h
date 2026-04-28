#ifndef UDP_TRANSPORT_H
#define UDP_TRANSPORT_H
#include "general/interfaces/io/io.h"

class UDPIOTransport : public IOTransport {
public:
  virtual Task<std::expected<int, ErrorWrapper>>
  initialize(IOAddress *fd_addr, IOAddress *socket_addr) = 0;

  virtual Task<std::expected<int, ErrorWrapper>>
  recv_from(IOAddress host_addr, IOAddress *out_addr, uint8_t *buf,
            uint64_t buf_size) = 0;
  virtual Task<std::expected<int, ErrorWrapper>> send_to(IOAddress host_addr,
                                                         IOAddress out_addr,
                                                         const uint8_t *buf,
                                                         uint64_t buf_size) = 0;

  virtual Task<std::expected<int, ErrorWrapper>> io_close(IOAddress addr) = 0;
};

#endif
