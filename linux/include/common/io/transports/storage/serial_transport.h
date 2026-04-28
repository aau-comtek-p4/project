#ifndef SERIAL_TRANSPORT_H
#define SERIAL_TRANSPORT_H

#include "common/io/io.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include <liburing.h>
class SerialIOTransport : public LibUringIO, public StorageIOTransport {

public:
  void cancel(const void *user_data) override;
  void submit() override;
  void process(uint64_t timeout) override;

  Task<std::expected<int, ErrorWrapper>> io_open(IOAddress addr) override;
  Task<std::expected<int, ErrorWrapper>> io_read(IOAddress addr, uint8_t *buf,
                                                 uint64_t buf_size) override;
  Task<std::expected<int, ErrorWrapper>>
  io_write(IOAddress addr, const uint8_t *buf, uint64_t buf_size) override;
  Task<std::expected<int, ErrorWrapper>> io_close(IOAddress addr) override;
};
#endif
