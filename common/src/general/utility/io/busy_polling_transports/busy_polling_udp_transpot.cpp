#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/busy_polling_awaiters.h"
#include "general/interfaces/io/transports/busy_pollings_transports.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/names.h"
#include <cstdint>
#include <expected>

Task<std::expected<int, ErrorWrapper>>
BusyPollingUDPTransport::initialize(const IOAddress *host_addr,
                                    uint64_t listen_backlog) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_BIND);
  self_ctxt->trace.start();

  int res = co_await busy_polling_awaiters::IOUDPOpenAwaiter(
      host_addr->val.sockaddr, listen_backlog);

  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::OPEN, res);
}

Task<std::expected<int, ErrorWrapper>> BusyPollingUDPTransport::send_to(
    const IOAddress *host_addr, const IOAddress *out_addr, const uint8_t *buf,
    uint64_t buf_size, IOPackageType package_type, bool with_ack) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_SEND);
  self_ctxt->trace.start();

  int res = co_await busy_polling_awaiters::IOSendToAwaiter(
      host_addr->val.fd, out_addr->val.sockaddr, buf, buf_size, package_type,
      with_ack);

  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::SEND, res);
}

Task<std::expected<int, ErrorWrapper>> BusyPollingUDPTransport::send_to(
    const IOAddress *host_addr, const IOAddress *out_addr, const uint8_t *buf,
    uint64_t buf_size, IOPackageType package_type) {
  return this->send_to(host_addr, out_addr, buf, buf_size, package_type, false);
}

Task<std::expected<int, ErrorWrapper>>
BusyPollingUDPTransport::recv_from(const IOAddress *host_addr,
                                   IOAddress *cli_addr, uint8_t *buf,
                                   uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_RECV);
  self_ctxt->trace.start();

  int res = co_await busy_polling_awaiters::IORecvFromAwaiter(
      host_addr->val.fd, cli_addr->val.sockaddr, buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::RECV, res);
}

Task<std::expected<int, ErrorWrapper>>
BusyPollingUDPTransport::io_close(const IOAddress *host_addr,
                                  const IOAddress *conn_addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_CLOSE);
  self_ctxt->trace.start();
  int res = co_await busy_polling_awaiters::IOUDPCloseAwaiter(
      host_addr->val.fd, conn_addr->val.sockaddr);
  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::CLOSE,
                           res);
}
