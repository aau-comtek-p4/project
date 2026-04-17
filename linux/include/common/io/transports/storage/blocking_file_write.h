#ifndef BLOCKING_FILE_WRITE_TRANSPORT_H
#define BLOCKING_FILE_WRITE_TRANSPORT_H

#include "general/interfaces/io/transports/storage_transport.h"
#include <cstdint>

class BlockingFileWriteIOTransport : public StorageIOTransport {
private:
public:
  BlockingFileWriteIOTransport(uint64_t queue_depth);
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
