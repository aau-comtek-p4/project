#ifndef STORAGE_TRANSPORT_H
#define STORAGE_TRANSPORT_H

#include "general/interfaces/io/io.h"
#include <cstdint>
#include <expected>
class StorageIOTransport : public IOTransport {
public:
public:
  virtual Task<std::expected<int, ErrorWrapper>> io_open(IOAddress addr) = 0;
  virtual Task<std::expected<int, ErrorWrapper>>
  io_read(IOAddress addr, uint8_t *buf, uint64_t buf_size) = 0;
  virtual Task<std::expected<int, ErrorWrapper>>
  io_write(IOAddress addr, const uint8_t *buf, uint64_t buf_size) = 0;
  virtual Task<std::expected<int, ErrorWrapper>> io_close(IOAddress addr) = 0;
};

#endif
