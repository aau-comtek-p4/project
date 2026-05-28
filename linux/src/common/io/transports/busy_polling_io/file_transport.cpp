
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/busy_polling_awaiters.h"
#include "general/interfaces/io/transports/busy_pollings_transports.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/names.h"
#include <cstdint>
#include <expected>
#include <fcntl.h>

Task<std::expected<int, ErrorWrapper>>
BusyPollingFileTransport::io_open(const IOAddress *host_addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_OPEN);
  self_ctxt->trace.start();

  int res = co_await busy_polling_awaiters::IOFileOpenAwaiter(
      program_ctxt->name_lookup->get_name(host_addr->val.file_name_index),
      O_RDWR | O_TRUNC | O_CREAT, 0644);

  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::OPEN, res);
}

Task<std::expected<int, ErrorWrapper>>
BusyPollingFileTransport::io_write(const IOAddress *host_addr,
                                   const uint8_t *buf, uint64_t buf_size,
                                   IOPackageType package_type, bool with_ack) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_WRITE);
  self_ctxt->trace.start();

  int res = co_await busy_polling_awaiters::IOFileWriteAwaiter(
      host_addr->val.fd, buf, buf_size);

  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::WRITE, res);
}

Task<std::expected<int, ErrorWrapper>>
BusyPollingFileTransport::io_write(const IOAddress *host_addr,
                                   const uint8_t *buf, uint64_t buf_size,
                                   IOPackageType package_type) {

  return this->io_write(host_addr, buf, buf_size, package_type, false);
}

Task<std::expected<int, ErrorWrapper>>
BusyPollingFileTransport::io_read(const IOAddress *host_addr, uint8_t *buf,
                                  uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_FILE_READ);
  self_ctxt->trace.start();

  int res = co_await busy_polling_awaiters::IOFileReadAwaiter(host_addr->val.fd,
                                                              buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::READ, res);
}

Task<std::expected<int, ErrorWrapper>>
BusyPollingFileTransport::io_close(const IOAddress *host_addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_SERIAL_CLOSE);
  self_ctxt->trace.start();
  int res =
      co_await busy_polling_awaiters::IOFileCloseAwaiter(host_addr->val.fd);
  co_return process_io_res(self_ctxt, IOMethod::IO_FILE, IOType::CLOSE, res);
}
