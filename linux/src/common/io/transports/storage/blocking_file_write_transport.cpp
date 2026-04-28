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
BlockingFileWriteIOTransport::BlockingFileWriteIOTransport() {}

void BlockingFileWriteIOTransport::submit() {}
void BlockingFileWriteIOTransport::process(uint64_t timeout) {}
void BlockingFileWriteIOTransport::cancel(const void *user_data) {}

Task<std::expected<int, ErrorWrapper>>
BlockingFileWriteIOTransport::io_open(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();

  self_ctxt->set_name(NAME_IO_FILE_OPEN);
  self_ctxt->trace.start();
  int res = open(addr.file_path, O_RDWR | O_CREAT | O_APPEND, 0644);

  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::OPEN, res);
}
Task<std::expected<int, ErrorWrapper>>
BlockingFileWriteIOTransport::io_read(IOAddress addr, uint8_t *buf,
                                      uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();

  self_ctxt->set_name(NAME_IO_FILE_READ);
  self_ctxt->trace.start();
  int res = read(addr.fd, buf, buf_size);

  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::READ, res);
}
Task<std::expected<int, ErrorWrapper>>
BlockingFileWriteIOTransport::io_write(IOAddress addr, const uint8_t *buf,
                                       uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();

  self_ctxt->set_name(NAME_IO_FILE_WRITE);
  self_ctxt->trace.start();

  int res = write(addr.fd, buf, buf_size);

  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::WRITE, res);
}
Task<std::expected<int, ErrorWrapper>>
BlockingFileWriteIOTransport::io_close(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_CLOSE);
  self_ctxt->trace.start();
  int res = close(addr.fd);

  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::CLOSE, res);
}
