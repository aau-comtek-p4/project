#include "common/io/io.h"
#include "common/io/transports/storage/liburing_file_write.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/shutdown.h"
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <liburing.h>
#include <liburing/io_uring.h>
class IOAwaiterInterface : public IOAwaitInterface {
public:
  io_uring *ring;
  int fd;
  int result;
  const char *get_type() { return parse_io_type(this->type); };
  virtual void submit(io_uring_sqe *sqe) = 0;
  bool await_ready() { return false; }
  bool await_suspend(std::coroutine_handle<
                     Task<std::expected<int, ErrorWrapper>>::promise_type>
                         handle) {
    if (handle.promise().ctxt.cancelled) {
      this->result = -ECANCELED;
      return false;
    }

    handle.promise().ctxt.io_address = this;
    this->handle = handle;
    struct io_uring_sqe *sqe = io_uring_get_sqe(this->ring);
    if (!sqe) {
      printf("NO SQE\n");
      safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
    }
    this->submit(sqe);
    io_uring_sqe_set_data(sqe, this);
    return true;
  }
  int await_resume() { return this->result; }
};

class IOAcceptAwaiter : public IOAwaiterInterface {

public:
  IOAcceptAwaiter(io_uring *ring, int fd) {
    this->ring = ring;
    this->fd = fd;
    this->type = IOType::ACCEPT;
    this->io_method = IOMethod::IO_WIFI_TCP;
  }
  void submit(io_uring_sqe *sqe) override {

    io_uring_prep_accept(sqe, this->fd, NULL, NULL, 0);
  }
};

class IORecvAwaiter : public IOAwaiterInterface {
private:
  uint8_t *buf;
  uint64_t buffer_size;

public:
  IORecvAwaiter(io_uring *ring, int fd, uint8_t *buf, uint64_t buffer_size) {
    this->ring = ring;
    this->fd = fd;
    this->buf = buf;
    this->buffer_size = buffer_size;
    this->type = IOType::RECV;
    this->io_method = IOMethod::IO_WIFI_TCP;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_recv(sqe, this->fd, this->buf, this->buffer_size, 0);
  }
};

class IOSendAwaiter : public IOAwaiterInterface {
private:
  const uint8_t *buf;
  uint64_t buffer_size;

public:
  IOSendAwaiter(io_uring *ring, int fd, const uint8_t *buf,
                uint64_t buffer_size) {
    this->ring = ring;
    this->fd = fd;
    this->buf = buf;
    this->buffer_size = buffer_size;
    this->type = IOType::SEND;
    this->io_method = IOMethod::IO_WIFI_TCP;
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
    this->io_method = IOMethod::IO_WIFI_TCP;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_connect(sqe, this->fd, (struct sockaddr *)&this->server_addr,
                          sizeof(this->server_addr));
  }
};

class IOSendToAwaiter : public IOAwaiterInterface {
private:
  sockaddr_in server_addr;
  const uint8_t *buf;
  uint64_t buffer_size;

public:
  IOSendToAwaiter(io_uring *ring, int fd, sockaddr_in server_addr,
                  const uint8_t *buf, uint64_t data_len) {
    this->ring = ring;
    this->fd = fd;
    this->server_addr = server_addr;
    this->type = IOType::CONNECT;
    this->buf = buf;
    this->buffer_size = data_len;
    this->io_method = IOMethod::IO_WIFI_UDP;
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
    this->io_method = IOMethod::IO_WIFI_UDP;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_recvmsg(sqe, this->fd, this->msg, 0);
  }
};
class IOReadAwaiter : public IOAwaiterInterface {
private:
  uint8_t *buf;
  uint64_t buffer_size;

public:
  IOReadAwaiter(io_uring *ring, int fd, uint8_t *out_buf, uint64_t max_read) {
    this->ring = ring;
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
  IOWriteAwaiter(io_uring *ring, int fd, const uint8_t *out_buf,
                 uint64_t max_read) {
    this->ring = ring;
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
  IOOpenAwaiter(io_uring *ring, const char *file_path, int flags, mode_t mode) {
    this->ring = ring;
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
  IOCloseAwaiter(io_uring *ring, int fd) {
    this->ring = ring;
    this->fd = fd;
    this->type = IOType::CLOSE;
    this->io_method = IOMethod::IO_FILE;
  }

  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_close(sqe, this->fd);
  }
};

LiburingFileWriteIOTransport::LiburingFileWriteIOTransport(
    uint64_t queue_depth) {
  io_uring_queue_init(queue_depth, &this->ring, 0);
}

void LiburingFileWriteIOTransport::submit() { io_uring_submit(&this->ring); }
void LiburingFileWriteIOTransport::process(uint64_t timeout) {
  io_uring_cqe *cqe;
  __kernel_timespec t{.tv_sec = 0, .tv_nsec = (uint32_t)timeout};
  io_uring_wait_cqe_timeout(&this->ring, &cqe, &t);
  uint64_t completed_count = io_uring_cq_ready(&this->ring);
  for (uint64_t i = 0; i < completed_count; i++) {
    io_uring_peek_cqe(&this->ring, &cqe);
    auto interface = (IOAwaiterInterface *)cqe->user_data;
    interface->result = cqe->res;
    io_uring_cqe_seen(&this->ring, cqe);
    interface->handle.resume();
  }
}
void LiburingFileWriteIOTransport::cancel(const void *user_data) {
  io_uring_sqe *sqe = io_uring_get_sqe(&this->ring);
  io_uring_prep_cancel(sqe, user_data, 0);
}

Task<std::expected<int, ErrorWrapper>>
LiburingFileWriteIOTransport::io_open(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_OPEN);
  self_ctxt->trace.start();
  int res = co_await IOOpenAwaiter(&this->ring, addr.file_path,
                                   O_RDWR | O_CREAT | O_APPEND, 0644);
  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::OPEN, res);
}
Task<std::expected<int, ErrorWrapper>>
LiburingFileWriteIOTransport::io_read(IOAddress addr, uint8_t *buf,
                                      uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_READ);
  self_ctxt->trace.start();
  int res = co_await IOReadAwaiter(&this->ring, addr.fd, buf, buf_size);

  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::READ, res);
}
Task<std::expected<int, ErrorWrapper>>
LiburingFileWriteIOTransport::io_write(IOAddress addr, const uint8_t *buf,
                                       uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_WRITE);
  self_ctxt->trace.start();
  int res = co_await IOWriteAwaiter(&this->ring, addr.fd, buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::WRITE, res);
}
Task<std::expected<int, ErrorWrapper>>
LiburingFileWriteIOTransport::io_close(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_CLOSE);
  self_ctxt->trace.start();
  int res = co_await IOCloseAwaiter(&this->ring, addr.fd);

  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::CLOSE, res);
}
