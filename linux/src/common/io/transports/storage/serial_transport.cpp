#include "common/io/transports/storage/serial_transport.h"
#include "common/io/io.h"
#include "general/misc/names.h"
#include <cstdint>
#include <fcntl.h>
#include <termios.h>
class IOSerialReadAwaiter : public IOAwaiterInterface {
private:
  uint8_t *buf;
  uint64_t buffer_size;

public:
  IOSerialReadAwaiter(int fd, uint8_t *out_buf, uint64_t max_read) {
    this->fd = fd;
    this->buf = out_buf;
    this->buffer_size = max_read;
    this->type = IOType::READ;
    this->io_method = IOMethod::IO_SERIAL;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_read(sqe, this->fd, this->buf, this->buffer_size, 0);
  }
};

class IOSerialWriteAwaiter : public IOAwaiterInterface {
private:
  const uint8_t *buf;
  uint64_t buffer_size;

public:
  IOSerialWriteAwaiter(int fd, const uint8_t *out_buf, uint64_t max_read) {
    this->fd = fd;
    this->buf = out_buf;
    this->buffer_size = max_read;
    this->type = IOType::WRITE;
    this->io_method = IOMethod::IO_SERIAL;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_write(sqe, this->fd, this->buf, this->buffer_size, -1);
  }
};

class IOSerialOpenAwaiter : public IOAwaiterInterface {
private:
  const char *path;
  int flags;
  mode_t mode;

public:
  IOSerialOpenAwaiter(const char *file_path, int flags, mode_t mode) {
    this->path = file_path;
    this->flags = flags;
    this->mode = mode;
    this->type = IOType::OPEN;
    this->io_method = IOMethod::IO_SERIAL;
  }

  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_open(sqe, this->path, this->flags, this->mode);
  }
};

class IOSerialCloseAwaiter : public IOAwaiterInterface {

public:
  IOSerialCloseAwaiter(int fd) {
    this->fd = fd;
    this->type = IOType::CLOSE;
    this->io_method = IOMethod::IO_SERIAL;
  }

  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_close(sqe, this->fd);
  }
};

void SerialIOTransport::submit() { return LibUringIO::submit(); }
void SerialIOTransport::process(uint64_t timeout) {

  return LibUringIO::process(timeout);
}
void SerialIOTransport::cancel(const void *user_data) {
  return LibUringIO::cancel(user_data);
}

Task<std::expected<int, ErrorWrapper>>
SerialIOTransport::io_open(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_SERIAL_OPEN);
  self_ctxt->trace.start();
  int res = co_await IOSerialOpenAwaiter(addr.file_path,
                                         O_RDWR | O_NOCTTY | O_NONBLOCK, 0644);
  auto res2 = process_io_res(self_ctxt, IOMethod::IO_SERIAL, IOType::OPEN, res);
  if (!res2.has_value()) {
    co_return res2;
  }

  struct termios tty;
  std::memset(&tty, 0, sizeof(tty));

  if (tcgetattr(res2.value(), &tty) != 0) {
    co_return std::unexpected(
        ErrorWrapper{.error = errno, .tag = ErrorWrapper::ERRNO});
  }
  // Put into raw mode (IMPORTANT)
  cfmakeraw(&tty);

  // Set baud rate (mostly symbolic for USB CDC, but required)
  cfsetispeed(&tty, UART_BAUDRATE);
  cfsetospeed(&tty, UART_BAUDRATE);

  // 8N1 (matches ESP32 config)
  tty.c_cflag &= ~PARENB; // no parity
  tty.c_cflag &= ~CSTOPB; // 1 stop bit
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8; // 8 data bits

  tty.c_cflag |= (CLOCAL | CREAD);
  tty.c_cflag &= ~CRTSCTS; // no flow control

  if (tcsetattr(res, TCSANOW, &tty) != 0) {
    co_return std::unexpected(
        ErrorWrapper{.error = errno, .tag = ErrorWrapper::ERRNO});
  }
  tcflush(res2.value(), TCIOFLUSH); // flush any garbage in buffers

  co_return res2;
}
Task<std::expected<int, ErrorWrapper>>
SerialIOTransport::io_read(IOAddress addr, uint8_t *buf, uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_SERIAL_READ);
  self_ctxt->trace.start();
  int res = co_await IOSerialReadAwaiter(addr.fd, buf, buf_size);

  co_return process_io_res(self_ctxt, IOMethod::IO_SERIAL, IOType::READ, res);
}
Task<std::expected<int, ErrorWrapper>>
SerialIOTransport::io_write(IOAddress addr, const uint8_t *buf,
                            uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_SERIAL_WRITE);
  self_ctxt->trace.start();
  int res = co_await IOSerialWriteAwaiter(addr.fd, buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_SERIAL, IOType::WRITE, res);
}
Task<std::expected<int, ErrorWrapper>>
SerialIOTransport::io_close(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_SERIAL_CLOSE);
  self_ctxt->trace.start();
  int res = co_await IOSerialCloseAwaiter(addr.fd);

  co_return process_io_res(self_ctxt, IOMethod::IO_SERIAL, IOType::CLOSE, res);
}
