#include <common/io/io.h>
const char *IOAwaiterInterface::get_type() {
  return parse_io_type(this->type);
};
bool IOAwaiterInterface::await_ready() { return false; }
bool IOAwaiterInterface::await_suspend(
    std::coroutine_handle<Task<std::expected<int, ErrorWrapper>>::promise_type>
        handle) {
  if (handle.promise().ctxt.cancelled) {
    this->result = -ECANCELED;
    return false;
  }

  handle.promise().ctxt.io_address = this;
  this->handle = handle;
  struct io_uring_sqe *sqe = io_uring_get_sqe(&program_uring);
  if (!sqe) {
    printf("NO SQE\n");
    safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
  }
  this->submit(sqe);
  io_uring_sqe_set_data(sqe, this);
  return true;
}
int IOAwaiterInterface::await_resume() { return this->result; }

void LibUringIO::submit() { io_uring_submit(&program_uring); }
void LibUringIO::process(uint64_t timeout) {
  io_uring_cqe *cqe;
  __kernel_timespec t{.tv_sec = 0, .tv_nsec = (uint32_t)timeout};
  io_uring_wait_cqe_timeout(&program_uring, &cqe, &t);
  uint64_t completed_count = io_uring_cq_ready(&program_uring);
  for (uint64_t i = 0; i < completed_count; i++) {
    io_uring_peek_cqe(&program_uring, &cqe);
    auto interface = (IOAwaiterInterface *)cqe->user_data;
    interface->result = cqe->res;
    io_uring_cqe_seen(&program_uring, cqe);
    interface->handle.resume();
  }
}
void LibUringIO::cancel(const void *user_data) {
  io_uring_sqe *sqe = io_uring_get_sqe(&program_uring);
  io_uring_prep_cancel(sqe, user_data, 0);
}
