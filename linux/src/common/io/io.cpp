#include "common/io/io.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
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
class IOAwaiterInterface : public IOAwaitInterface {
public:
  io_uring *ring;
  int fd;
  uint8_t *buf;
  size_t buffer_size;
  int result;
  IOType type;
  void set_result(int result) { this->result = result; }
  const char *get_type() { return get_io_type(this->type); };

  virtual void submit(io_uring_sqe *sqe) = 0;
  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<
                     Task<std::expected<int, ErrorWrapper>>::promise_type>
                         handle) {
    if (handle.promise().cancelled) {
      this->result = -ECANCELED;
      auto res = program_ctxt->loop->enque_staging(handle);
      if (!res.has_value()) {
        program_ctxt->logger->log_err(IO_TAG,
                                      "Cancelled IO failed to enque handle");
        safe_shutdown(res.error());
      }
      return;
    }

    handle.promise().io_address = this;
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

Task<std::expected<int, ErrorWrapper>> LinuxIO::read(int id, uint8_t *out_buf,
                                                     size_t max_read) {
  int result = co_await IOReadAwaiter(&this->ring, id, out_buf, max_read);
  if (result < 0) {
    program_ctxt->metrics->document_metric(MetricType::READ_FAILED);
    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = result * -1});
  }
  program_ctxt->metrics->document_metric(MetricType::FILE_READ);
  co_return result;
}

Task<std::expected<int, ErrorWrapper>> LinuxIO::write(int id, uint8_t *in_buf,
                                                      size_t write_amount) {
  int result = co_await IOWriteAwaiter(&this->ring, id, in_buf, write_amount);

  if (result < 0) {
    program_ctxt->metrics->document_metric(MetricType::WRITE_FAILED);
    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = result * -1});
  }
  program_ctxt->metrics->document_metric(MetricType::FILE_WRITE);
  co_return result;
}
Task<std::expected<int, ErrorWrapper>> LinuxIO::open(const char *file_path,
                                                     int flags, mode_t mode) {
  int result = co_await IOOpenAwaiter(&this->ring, file_path, flags, mode);

  if (result < 0) {
    program_ctxt->metrics->document_metric(MetricType::FAILED_OPENED);
    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = result * -1});
  }
  program_ctxt->metrics->document_metric(MetricType::OPENED_FILE);
  co_return result;
}

Task<std::expected<int, ErrorWrapper>> LinuxIO::close(int fd) {
  int result = co_await IOCloseAwaiter(&this->ring, fd);

  if (result < 0) {
    program_ctxt->metrics->document_metric(MetricType::CLOSED_FAILED);
    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = result * -1});
  }
  program_ctxt->metrics->document_metric(MetricType::CLOSED_FD);
  co_return result;
};
Task<std::expected<int, ErrorWrapper>> LinuxIO::accept(int fd) {
  int result = co_await IOAcceptAwaiter(&this->ring, fd);
  if (result < 0) {
    program_ctxt->metrics->document_metric(MetricType::FAILED_ACCEPT);

    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = result * -1});
  }
  program_ctxt->metrics->document_metric(MetricType::ACCEPTED_CONNECTION);
  co_return result;
}

Task<std::expected<int, ErrorWrapper>> LinuxIO::send(int fd, uint8_t *buf,
                                                     size_t buffer_size) {
  int result = co_await IOSendAwaiter(&this->ring, fd, buf, buffer_size);
  if (result < 0) {
    program_ctxt->metrics->document_metric(MetricType::FAILED_SEND);
    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = result * -1});
  }
  program_ctxt->metrics->document_metric(MetricType::MESSAGE_SENT);
  co_return result;
}

Task<std::expected<int, ErrorWrapper>> LinuxIO::recv(int fd, uint8_t *buf,
                                                     size_t buffer_size) {
  int result = co_await IORecvAwaiter(&this->ring, fd, buf, buffer_size);
  if (result < 0) {
    program_ctxt->metrics->document_metric(MetricType::FAILED_RECEIVE);

    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = result * -1});
  }
  if (result == 0) {
    program_ctxt->metrics->document_metric(MetricType::DISCONNECT);
    co_return result;
  }
  program_ctxt->metrics->document_metric(MetricType::MESSAGE_RECEIVED);
  co_return result;
}

Task<std::expected<int, ErrorWrapper>>
LinuxIO::connect(int fd, sockaddr_in server_addr) {
  int result = co_await IOConnectAwaiter(&this->ring, fd, server_addr);
  if (result < 0) {

    program_ctxt->metrics->document_metric(MetricType::FAILED_CONNECT);
    co_return std::unexpected(
        ErrorWrapper{.tag = ErrorWrapper::ERRNO, .error = result * -1});
  }

  program_ctxt->metrics->document_metric(MetricType::CONNECTION_RECEIVED);
  co_return result;
}
void LinuxIO::submit() {
  program_ctxt->logger->log_debug(IO_TAG, "Submitted IO");
  io_uring_submit(&this->ring);
}
void LinuxIO::cancel(const void *user_data) {
  io_uring_sqe *sqe = io_uring_get_sqe(&this->ring);
  io_uring_prep_cancel(sqe, user_data, 0);
  io_uring_sqe_set_data(sqe, nullptr);
}

void LinuxIO::process_cqe(uint64_t timeout_ns) {
  program_ctxt->logger->log_debug(IO_TAG, "Processing cqe");
  io_uring_cqe *cqe;

  uint32_t tv_sec = timeout_ns / (NS_PR_MS * MS_PR_S);
  uint32_t tv_nsec = timeout_ns % (NS_PR_MS * MS_PR_S);
  struct __kernel_timespec ts{.tv_sec = tv_sec, .tv_nsec = tv_nsec};

  program_ctxt->logger->log_debug(
      IO_TAG, "Setting cqe timeout, timeout ns: [%lu],sec: [%u], ns: [%u]",
      timeout_ns, tv_sec, tv_nsec);
  io_uring_wait_cqe_timeout(&this->ring, &cqe, &ts);
  uint64_t head;
  size_t count = 0;
  io_uring_for_each_cqe(&this->ring, head, cqe) {
    auto awaiter =
        static_cast<IOAwaiterInterface *>(io_uring_cqe_get_data(cqe));
    if (!awaiter) {
      count += 1;
      continue;
    }

    program_ctxt->logger->log_debug(IO_TAG, "CQE RES: [%i]", cqe->res);

    awaiter->set_result(cqe->res);

    program_ctxt->logger->log_debug(IO_TAG,
                                    "IO finished, type: [%s], res: [%i]",
                                    awaiter->get_type(), cqe->res);
    auto res = program_ctxt->loop->enque_staging(awaiter->handle);
    if (!res.has_value()) {
      program_ctxt->logger->log_err(IO_ERROR_TAG,
                                    "Failed to enque cqe handler, error: [%s]",
                                    custom_strerror(res.error()));
      safe_shutdown(res.error());
    }
    count += 1;
  }

  if (count == 0) {
    program_ctxt->logger->log_debug(IO_TAG, "No CQE in queue");
  }
  io_uring_cq_advance(&this->ring, count);
}
