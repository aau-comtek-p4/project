#include "common/io/io.h"
#include "common/io/transports/storage/blocking_file_write.h"
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cerrno>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <fcntl.h>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
class IOAwaiterInterface : public IOAwaitInterface {
public:
  io_uring *ring;
  int fd;
  uint8_t *buf;
  uint64_t buffer_size;
  int result;
  IOType type;
  void set_result(int result) { this->result = result; }
  const char *get_type() { return parse_io_type(this->type); };

  virtual void submit(io_uring_sqe *sqe) = 0;
  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<
                     Task<std::expected<int, ErrorWrapper>>::promise_type>
                         handle) {
    if (handle.promise().ctxt.cancelled) {
      this->result = -ECANCELED;
      auto res = program_ctxt->loop->enque_staging(handle);
      if (!res.has_value()) {
        safe_shutdown(res.error());
      }
      return;
    }

    handle.promise().ctxt.io_address = this;
    this->handle = handle;
    struct io_uring_sqe *sqe = io_uring_get_sqe(this->ring);
    this->submit(sqe);
    io_uring_sqe_set_data(sqe, this);
  }
  int await_resume() { return this->result; }
};

class IOReadAwaiter : public IOAwaiterInterface {
public:
  IOReadAwaiter(io_uring *ring, int fd, uint8_t *out_buf, uint64_t max_read) {
    this->ring = ring;
    this->fd = fd;
    this->buf = out_buf;
    this->buffer_size = max_read;
    this->type = IOType::READ;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_read(sqe, this->fd, this->buf, this->buffer_size, 0);
  }
};

class IOWriteAwaiter : public IOAwaiterInterface {
public:
  IOWriteAwaiter(io_uring *ring, int fd, uint8_t *out_buf, uint64_t max_read) {
    this->ring = ring;
    this->fd = fd;
    this->buf = out_buf;
    this->buffer_size = max_read;
    this->type = IOType::WRITE;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_write(sqe, this->fd, this->buf, this->buffer_size, 0);
  }
};

class IOOpenAwaiter : public IOAwaiterInterface {
private:
  const char *path;
  int flags;
  mode_t mode;

public:
  IOOpenAwaiter(io_uring *ring, const char *file_path, int flags, mode_t mode) {
    this->ring = ring;
    this->path = file_path;
    this->flags = flags;
    this->mode = mode;
    this->type = IOType::OPEN;
  }

  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_open(sqe, this->path, this->flags, this->mode);
  }
};

class IOCloseAwaiter : public IOAwaiterInterface {

public:
  IOCloseAwaiter(io_uring *ring, int fd) {
    this->ring = ring;
    this->fd = fd;
    this->type = IOType::CLOSE;
  }

  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_close(sqe, this->fd);
  }
};

class IOAcceptAwaiter : public IOAwaiterInterface {

public:
  IOAcceptAwaiter(io_uring *ring, int fd) {
    this->ring = ring;
    this->fd = fd;
    this->type = IOType::ACCEPT;
  }
  void submit(io_uring_sqe *sqe) override {

    io_uring_prep_accept(sqe, this->fd, NULL, NULL, 0);
  }
};

class IORecvAwaiter : public IOAwaiterInterface {
public:
  IORecvAwaiter(io_uring *ring, int fd, uint8_t *buf, uint64_t buffer_size) {
    this->ring = ring;
    this->fd = fd;
    this->buf = buf;
    this->buffer_size = buffer_size;
    this->type = IOType::RECV;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_recv(sqe, this->fd, this->buf, this->buffer_size, 0);
  }
};

class IOSendAwaiter : public IOAwaiterInterface {
public:
  IOSendAwaiter(io_uring *ring, int fd, uint8_t *buf, uint64_t buffer_size) {
    this->ring = ring;
    this->fd = fd;
    this->buf = buf;
    this->buffer_size = buffer_size;
    this->type = IOType::SEND;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_send(sqe, this->fd, this->buf, this->buffer_size, 0);
  }
};

class IOConnectAwaiter : public IOAwaiterInterface {
private:
  sockaddr_in server_addr;

public:
  IOConnectAwaiter(io_uring *ring, int fd, sockaddr_in server_addr) {
    this->ring = ring;
    this->fd = fd;
    this->server_addr = server_addr;
    this->type = IOType::CONNECT;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_connect(sqe, this->fd, (struct sockaddr *)&this->server_addr,
                          sizeof(this->server_addr));
  }
};

class IOSendToAwaiter : public IOAwaiterInterface {
private:
  sockaddr_in server_addr;

public:
  IOSendToAwaiter(io_uring *ring, int fd, sockaddr_in server_addr, uint8_t *buf,
                  uint64_t data_len) {
    this->ring = ring;
    this->fd = fd;
    this->server_addr = server_addr;
    this->type = IOType::CONNECT;
    this->buf = buf;
    this->buffer_size = data_len;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_sendto(sqe, this->fd, this->buf, this->buffer_size, 0,
                         (sockaddr *)&this->server_addr, sizeof(sockaddr_in));
  }
};

class IORecvFromAwaiter : public IOAwaiterInterface {
private:
  msghdr *msg;

public:
  IORecvFromAwaiter(io_uring *ring, int fd, msghdr *msg) {
    this->ring = ring;
    this->fd = fd;
    this->type = IOType::CONNECT;
    this->msg = msg;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_recvmsg(sqe, this->fd, this->msg, 0);
  }
};
BlockingFileWriteIOTransport::BlockingFileWriteIOTransport(
    uint64_t queue_depth) {}

void BlockingFileWriteIOTransport::submit() {}
void BlockingFileWriteIOTransport::process(uint64_t timeout) {}
void BlockingFileWriteIOTransport::cancel(const void *user_data) {}

Task<std::expected<int, ErrorWrapper>>
BlockingFileWriteIOTransport::io_open(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();

  self_ctxt->set_name(4);
  self_ctxt->trace->start();
  int res = open(addr.file_path, O_RDWR | O_CREAT | O_APPEND, 0644);

  if (res < 0) {
    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = errno});
  }
  co_return res;
}
Task<std::expected<int, ErrorWrapper>>
BlockingFileWriteIOTransport::io_read(IOAddress addr, uint8_t *buf,
                                      uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();

  self_ctxt->set_name(7);
  self_ctxt->trace->start();
  int res = read(addr.fd, buf, buf_size);
  if (res < 0) {
    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = errno});
  }
  co_return res;
}
Task<std::expected<int, ErrorWrapper>>
BlockingFileWriteIOTransport::io_write(IOAddress addr, const uint8_t *buf,
                                       uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();

  self_ctxt->set_name(3);
  self_ctxt->trace->start();

  int res = write(addr.fd, buf, buf_size);
  if (res < 0) {
    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = errno});
  }
  co_return res;
}
Task<std::expected<int, ErrorWrapper>>
BlockingFileWriteIOTransport::io_close(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(5);
  self_ctxt->trace->start();
  int res = close(addr.fd);
  if (res < 0) {
    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = errno});
  }
  co_return res;
}
