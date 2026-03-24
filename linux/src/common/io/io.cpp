#include "common/io/io.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <algorithm>
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
const char *get_io_type(IOType type) {
  switch (type) {
  case IOType::OPEN:
    return "OPEN";
  case IOType::READ:
    return "READ";
  case IOType::WRITE:
    return "WRITE";
  case IOType::CLOSE:
    return "CLOSE";
  case IOType::RECV:
    return "RECV";
  case IOType::ACCEPT:
    return "ACCEPT";
  case IOType::SEND:
    return "SEND";
  case IOType::CONNECT:
    return "CONNECT";
  }
  return "Unknown type";
}
class IOAwaiterInterface {
public:
  io_uring *ring;
  int fd;
  uint8_t *buf;
  size_t buffer_size;
  std::coroutine_handle<> handle;
  int result;
  IOType type;
  void set_result(int result) { this->result = result; }
  const char *get_type() { return get_io_type(this->type); };

  virtual void submit(io_uring_sqe *sqe) = 0;
  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<> handle) {
    this->handle = handle;
    struct io_uring_sqe *sqe = io_uring_get_sqe(this->ring);

    this->submit(sqe);
    io_uring_sqe_set_data(sqe, this);
  }
  int await_resume() { return this->result; }
};

class IOReadAwaiter : public IOAwaiterInterface {
public:
  IOReadAwaiter(io_uring *ring, int fd, uint8_t *out_buf, size_t max_read) {
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
  IOWriteAwaiter(io_uring *ring, int fd, uint8_t *out_buf, size_t max_read) {
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
  IORecvAwaiter(io_uring *ring, int fd, uint8_t *buf, size_t buffer_size) {
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
  IOSendAwaiter(io_uring *ring, int fd, uint8_t *buf, size_t buffer_size) {
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

LinuxIO::LinuxIO(size_t queue_depth) : queue_depth(queue_depth) {
  io_uring_queue_init(this->queue_depth, &this->ring, 0);
}

Task<std::expected<int, IORes>> LinuxIO::read(int id, uint8_t *out_buf,
                                              size_t max_read) {
  int result = co_await IOReadAwaiter(&this->ring, id, out_buf, max_read);
  if (result < 0) {
    co_return std::unexpected(IORes{.cust_error = ReadError::READ_FAILED,
                                    .error_number = result * -1});
  }
  co_return result;
}

Task<std::expected<int, IORes>> LinuxIO::write(int id, uint8_t *in_buf,
                                               size_t write_amount) {
  int result = co_await IOWriteAwaiter(&this->ring, id, in_buf, write_amount);

  if (result < 0) {
    co_return std::unexpected(IORes{.cust_error = WriteError::WRITE_FAILED,
                                    .error_number = result * -1});
  }
  co_return result;
}
Task<std::expected<int, IORes>> LinuxIO::open(const char *file_path, int flags,
                                              mode_t mode) {
  int result = co_await IOOpenAwaiter(&this->ring, file_path, flags, mode);

  if (result < 0) {
    co_return std::unexpected(IORes{.cust_error = OpenError::OPEN_FAILED,
                                    .error_number = result * -1});
  }
  co_return result;
}

Task<std::expected<int, IORes>> LinuxIO::close(int fd) {
  int result = co_await IOCloseAwaiter(&this->ring, fd);

  if (result < 0) {
    co_return std::unexpected(
        IORes{.cust_error = 1, .error_number = result * -1});
  }
  co_return result;
}

Task<std::expected<int, IORes>> LinuxIO::accept(int fd) {
  int result = co_await IOAcceptAwaiter(&this->ring, fd);
  if (result < 0) {

    co_return std::unexpected(
        IORes{.cust_error = ConnectionError::CONNECTION_FAILED,
              .error_number = result * -1});
  }
  co_return result;
}

Task<std::expected<int, IORes>> LinuxIO::send(int fd, uint8_t *buf,
                                              size_t buffer_size) {
  int result = co_await IOSendAwaiter(&this->ring, fd, buf, buffer_size);
  if (result < 0) {

    co_return std::unexpected(IORes{.cust_error = SendError::SEND_FAILED,
                                    .error_number = result * -1});
  }
  co_return result;
}

Task<std::expected<int, IORes>> LinuxIO::recv(int fd, uint8_t *buf,
                                              size_t buffer_size) {
  int result = co_await IORecvAwaiter(&this->ring, fd, buf, buffer_size);
  if (result < 0) {

    co_return std::unexpected(IORes{.cust_error = ReceiveError::RECEIVED_FAILED,
                                    .error_number = result * -1});
  }
  co_return result;
}

Task<std::expected<int, IORes>> LinuxIO::connect(int fd,
                                                 sockaddr_in server_addr) {
  int result = co_await IOConnectAwaiter(&this->ring, fd, server_addr);
  if (result < 0) {

    co_return std::unexpected(
        IORes{.cust_error = ConnectionError::CONNECTION_FAILED,
              .error_number = result * -1});
  }
  co_return result;
}
void LinuxIO::submit() { io_uring_submit(&this->ring); }

void LinuxIO::process_cqe(uint64_t timeout_ns) {
  program_logger->log_debug(IO_TAG, "Processing cqe");
  io_uring_cqe *cqe;

  uint32_t tv_sec = timeout_ns / (NS_PR_MS * MS_PR_S);
  uint32_t tv_nsec = timeout_ns % (NS_PR_MS * MS_PR_S);
  struct __kernel_timespec ts{.tv_sec = tv_sec, .tv_nsec = tv_nsec};

  program_logger->log_debug(
      IO_TAG, "Setting cqe timeout, timeout ns: [%lu],sec: [%u], ns: [%u]",
      timeout_ns, tv_sec, tv_nsec);
  io_uring_wait_cqe_timeout(&this->ring, &cqe, &ts);
  uint64_t head;
  size_t count = 0;
  io_uring_for_each_cqe(&this->ring, head, cqe) {
    auto awaiter =
        static_cast<IOAwaiterInterface *>(io_uring_cqe_get_data(cqe));
    awaiter->set_result(cqe->res);

    program_logger->log_debug(IO_TAG, "IO finished, type: [%s], res: [%i]",
                              awaiter->get_type(), cqe->res);
    auto res = program_loop->enque_staging(awaiter->handle);
    if (!res.has_value()) {
      program_logger->log_err(IO_ERROR_TAG,
                              "Failed to enque cqe handler, error: [%s]",
                              custom_strerror(res.error()));
      safe_shutdown(res.error());
    }
    count += 1;
  }

  if (count == 0) {
    program_logger->log_debug(IO_TAG, "No CQE in queue");
  }
  io_uring_cq_advance(&this->ring, count);
}
