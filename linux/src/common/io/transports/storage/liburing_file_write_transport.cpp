#include "common/io/io.h"
#include "common/io/transports/storage/liburing_file_write.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/shutdown.h"
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <liburing.h>
#include <liburing/io_uring.h>

class IOReadAwaiter : public IOAwaiterInterface {
private:
  uint8_t *buf;
  uint64_t buffer_size;

public:
  IOReadAwaiter(int fd, uint8_t *out_buf, uint64_t max_read) {
    this->fd = fd;
    this->buf = out_buf;
    this->buffer_size = max_read;
    this->type = IOType::READ;
    this->io_method = IOMethod::IO_FILE;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_read(sqe, this->fd, this->buf, this->buffer_size, 0);
  }
};

class IOWriteAwaiter : public IOAwaiterInterface {
private:
  const uint8_t *buf;
  uint64_t buffer_size;

public:
  IOWriteAwaiter(int fd, const uint8_t *out_buf, uint64_t max_read) {
    this->fd = fd;
    this->buf = out_buf;
    this->buffer_size = max_read;
    this->type = IOType::WRITE;
    this->io_method = IOMethod::IO_FILE;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_write(sqe, this->fd, this->buf, this->buffer_size, -1);
  }
};

class IOOpenAwaiter : public IOAwaiterInterface {
private:
  const char *path;
  int flags;
  mode_t mode;

public:
  IOOpenAwaiter(const char *file_path, int flags, mode_t mode) {
    this->path = file_path;
    this->flags = flags;
    this->mode = mode;
    this->type = IOType::OPEN;
    this->io_method = IOMethod::IO_FILE;
  }

  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_open(sqe, this->path, this->flags, this->mode);
  }
};

class IOCloseAwaiter : public IOAwaiterInterface {

public:
  IOCloseAwaiter(int fd) {
    this->fd = fd;
    this->type = IOType::CLOSE;
    this->io_method = IOMethod::IO_FILE;
  }

  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_close(sqe, this->fd);
  }
};
void LiburingFileWriteIOTransport::cancel(const void *user_data) {
  return LibUringIO::cancel(user_data);
};
void LiburingFileWriteIOTransport::submit() { return LibUringIO::submit(); };
void LiburingFileWriteIOTransport::process(uint64_t timeout) {
  return LibUringIO::process(timeout);
};

Task<std::expected<int, ErrorWrapper>>
LiburingFileWriteIOTransport::io_open(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_OPEN);
  self_ctxt->trace.start();
  int res =
      co_await IOOpenAwaiter(addr.file_path, O_RDWR | O_CREAT | O_TRUNC, 0644);
  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::OPEN, res);
}
Task<std::expected<int, ErrorWrapper>>
LiburingFileWriteIOTransport::io_read(IOAddress addr, uint8_t *buf,
                                      uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_READ);
  self_ctxt->trace.start();
  int res = co_await IOReadAwaiter(addr.fd, buf, buf_size);

  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::READ, res);
}
Task<std::expected<int, ErrorWrapper>>
LiburingFileWriteIOTransport::io_write(IOAddress addr, const uint8_t *buf,
                                       uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_WRITE);
  self_ctxt->trace.start();
  int res = co_await IOWriteAwaiter(addr.fd, buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::WRITE, res);
}
Task<std::expected<int, ErrorWrapper>>
LiburingFileWriteIOTransport::io_close(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_CLOSE);
  self_ctxt->trace.start();
  int res = co_await IOCloseAwaiter(addr.fd);

  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::CLOSE, res);
}
