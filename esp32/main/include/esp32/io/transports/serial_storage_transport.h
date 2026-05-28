#ifndef ESP_SERIAL_TRANSPORT_H
#define ESP_SERIAL_TRANSPORT_H
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/storage_transport.h"

class ESPSerialTransport : public StorageIOTransport {
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
