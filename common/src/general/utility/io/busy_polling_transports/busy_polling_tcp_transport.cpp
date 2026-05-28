#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/busy_polling_awaiters.h"
#include "general/interfaces/io/transports/busy_pollings_transports.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/names.h"
#include <cstdint>
#include <expected>

Task<std::expected<int, ErrorWrapper>> BusyPollingTCPTransport::initialize(
    const IOAddress *host_addr, uint64_t listen_backlog, IOTCPType tcp_type) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_TCP_BIND);
  self_ctxt->trace.start();

  int res = co_await busy_polling_awaiters::IOTCPOpenAwaiter(
      host_addr->val.sockaddr, listen_backlog, tcp_type);

  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_TCP, IOType::OPEN, res);
}
Task<std::expected<int, ErrorWrapper>>
BusyPollingTCPTransport::accept(const IOAddress *host_addr,
                                IOAddress *cli_addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_TCP_ACCEPT);
  self_ctxt->trace.start();

  int res = co_await busy_polling_awaiters::IOAcceptAwaiter(
      host_addr->val.fd, cli_addr->val.sockaddr);

  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_TCP, IOType::ACCEPT,
                           res);
}

Task<std::expected<int, ErrorWrapper>>
BusyPollingTCPTransport::send(const IOAddress *host_addr, const uint8_t *buf,
                              uint64_t buf_size, IOPackageType package_type,
                              bool with_ack) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_TCP_SEND);
  self_ctxt->trace.start();

  int res = co_await busy_polling_awaiters::IOSendAwaiter(
      host_addr->val.fd, buf, buf_size, package_type, with_ack);

  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_TCP, IOType::SEND, res);
}

Task<std::expected<int, ErrorWrapper>>
BusyPollingTCPTransport::send(const IOAddress *host_addr, const uint8_t *buf,
                              uint64_t buf_size, IOPackageType package_type) {
  return this->send(host_addr, buf, buf_size, package_type, false);
}

Task<std::expected<int, ErrorWrapper>>
BusyPollingTCPTransport::recv(const IOAddress *host_addr, uint8_t *buf,
                              uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_TCP_RECV);
  self_ctxt->trace.start();

  int res = co_await busy_polling_awaiters::IORecvAwaiter(host_addr->val.fd,
                                                          buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_TCP, IOType::RECV, res);
}

Task<std::expected<int, ErrorWrapper>>
BusyPollingTCPTransport::connect(const IOAddress *host_addr,
                                 const IOAddress *connect_addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_TCP_CONNECT);
  self_ctxt->trace.start();

  int res = co_await busy_polling_awaiters::IOConnectAwaiter(
      host_addr->val.fd, connect_addr->val.sockaddr);
  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_TCP, IOType::CONNECT,
                           res);
}

Task<std::expected<int, ErrorWrapper>>
BusyPollingTCPTransport::io_close(const IOAddress *host_addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_TCP_CLOSE);
  self_ctxt->trace.start();
  int res =
      co_await busy_polling_awaiters::IOTCPCloseAwaiter(host_addr->val.fd);
  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_TCP, IOType::CLOSE,
                           res);
}
