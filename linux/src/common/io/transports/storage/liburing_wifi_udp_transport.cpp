#include "common/io/transports/network/liburing_wifi_udp_transport.h"
#include "common/io/io.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/names.h"
#include <liburing.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/uio.h>

class IOSendToAwaiter : public IOAwaiterInterface {
private:
  sockaddr_in server_addr;
  const uint8_t *buf;
  uint64_t buffer_size;

public:
  IOSendToAwaiter(int fd, sockaddr_in server_addr, const uint8_t *buf,
                  uint64_t data_len) {
    this->fd = fd;
    this->server_addr = server_addr;
    this->type = IOType::SEND;
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
  IORecvFromAwaiter(int fd, msghdr *msg) {
    this->fd = fd;
    this->type = IOType::RECV;
    this->msg = msg;
    this->io_method = IOMethod::IO_WIFI_UDP;
  }
  void submit(io_uring_sqe *sqe) override {
    io_uring_prep_recvmsg(sqe, this->fd, this->msg, 0);
  }
};

void LiburingWIFIUDPTransport::cancel(const void *user_data) {
  return LibUringIO::cancel(user_data);
};
void LiburingWIFIUDPTransport::submit() { return LibUringIO::submit(); };
void LiburingWIFIUDPTransport::process(uint64_t timeout) {
  return LibUringIO::process(timeout);
};

Task<std::expected<int, ErrorWrapper>>
LiburingWIFIUDPTransport::initialize(IOAddress *fd_addr, IOAddress *host_addr) {

  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_BIND);
  self_ctxt->trace.start();
  int listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (listen_fd < 0) {
    co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::OPEN,
                             listen_fd);
  }
  auto res = bind(listen_fd, (struct sockaddr *)&host_addr->sockaddr,
                  sizeof(host_addr->sockaddr));
  if (res < 0) {
    co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::OPEN,
                             res);
  }
  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::OPEN,
                           listen_fd);
}

Task<std::expected<int, ErrorWrapper>>
LiburingWIFIUDPTransport::recv_from(IOAddress host_addr, IOAddress *out_addr,
                                    uint8_t *buf, uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_RECV);
  self_ctxt->trace.start();
  iovec iov;
  iov.iov_base = buf;
  iov.iov_len = buf_size;
  msghdr msg = {0};
  msg.msg_name = &out_addr->sockaddr;
  msg.msg_namelen = sizeof(out_addr->sockaddr);

  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  auto res = co_await IORecvFromAwaiter(host_addr.fd, &msg);
  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::RECV, res);
}

Task<std::expected<int, ErrorWrapper>>
LiburingWIFIUDPTransport::send_to(IOAddress host_addr, IOAddress out_addr,
                                  const uint8_t *buf, uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_SEND);
  self_ctxt->trace.start();
  auto res =
      co_await IOSendToAwaiter(host_addr.fd, out_addr.sockaddr, buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::SEND, res);
}

Task<std::expected<int, ErrorWrapper>>
LiburingWIFIUDPTransport::io_close(IOAddress fd_addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_CLOSE);
  self_ctxt->trace.start();

  auto res = close(fd_addr.fd);

  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::CLOSE,
                           res);
}
