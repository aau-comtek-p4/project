#ifndef STORAGE_TRANSPORT_H
#define STORAGE_TRANSPORT_H

#include "general/interfaces/io/io.h"
#include <cstdint>
#include <expected>
class StorageIOTransport : public IOTransport {
public:
  virtual Task<std::expected<int, ErrorWrapper>>
  io_open(const IOAddress *addr) = 0;
  virtual Task<std::expected<int, ErrorWrapper>>
  io_read(const IOAddress *addr, uint8_t *buf, uint64_t buf_size) = 0;

  virtual Task<std::expected<int, ErrorWrapper>>
  io_write(const IOAddress *addr, const uint8_t *buf, uint64_t buf_size,
           IOPackageType package_type) = 0;

  virtual Task<std::expected<int, ErrorWrapper>>
  io_write(const IOAddress *addr, const uint8_t *buf, uint64_t buf_size,
           IOPackageType package_type, bool with_ack) = 0;

  virtual Task<std::expected<int, ErrorWrapper>>
  io_close(const IOAddress *addr) = 0;
};

#endif
