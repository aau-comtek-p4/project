#include "esp32/io/transports/wifi_udp_transport.h"
#include "driver/uart.h"
#include "esp32/io/io.h"
#include "esp32/io/polling/wifi_udp_polling.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"
#include "lwip/sockets.h"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <netinet/in.h>
#include <sys/_default_fcntl.h>
#include <sys/socket.h>
class IOSendToAwaiter : public ESPIOAwaiterInterface {
private:
  int fd;
  const uint8_t *buf;
  uint64_t buffer_size;
  sockaddr_in send_addr;

public:
  IOSendToAwaiter(int fd, sockaddr_in send_addr, const uint8_t *buf,
                  uint64_t buffer_size) {
    this->fd = fd;
    this->send_addr = send_addr;
    this->buf = buf;
    this->buffer_size = buffer_size;
    this->type = IOType::SEND;
    this->io_method = IOMethod::IO_WIFI_UDP;
  }
  void submit() override {
    esp_io::prep_sqe_wifi_udp_send_to(this->fd, this->send_addr, this->buf,
                                      this->buffer_size, this);
    ;
  }
};

class IORecvFromAwaiter : public ESPIOAwaiterInterface {
private:
  int fd;
  uint8_t *buf;
  uint64_t buffer_size;
  sockaddr_in *recv_addr;
  socklen_t *sock_size;

public:
  IORecvFromAwaiter(int fd, sockaddr_in *recv_addr, socklen_t *sock_size,
                    uint8_t *buf, uint64_t buffer_size) {

    this->fd = fd;
    this->sock_size = sock_size;
    this->recv_addr = recv_addr;
    this->buf = buf;
    this->buffer_size = buffer_size;
    this->type = IOType::RECV;
    this->io_method = IOMethod::IO_WIFI_UDP;
  }
  void submit() override {
    esp_io::prep_sqe_wifi_udp_recv_from(this->fd, this->recv_addr, sock_size,
                                        this->buf, this->buffer_size, this);
    ;
  }
};

void ESPWIFIUDPTransport::cancel(const void *user_data) {};
void ESPWIFIUDPTransport::submit() {};
void ESPWIFIUDPTransport::process(uint64_t timeout) {};

Task<std::expected<int, ErrorWrapper>>
ESPWIFIUDPTransport::initialize(IOAddress *fd_addr, IOAddress *host_addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_BIND);
  self_ctxt->trace.start();
  int listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (listen_fd < 0) {
    co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::OPEN,
                             listen_fd);
  }
  fd_addr->fd = listen_fd;
  int f_res = fcntl(listen_fd, F_SETFL, O_NONBLOCK);
  if (f_res < 0) {
    co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::OPEN,
                             f_res);
  }
  auto res = bind(listen_fd, (struct sockaddr *)&host_addr->sockaddr,
                  sizeof(host_addr->sockaddr));
  if (res < 0) {
    co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::OPEN,
                             res);
  }

  auto res2 = wifi_udp_tracker.register_fd(listen_fd);

  if (!res2.has_value()) {
    co_return std::unexpected(res2.error());
  }
  co_return 0;
}

Task<std::expected<int, ErrorWrapper>>
ESPWIFIUDPTransport::recv_from(IOAddress host_addr, IOAddress *out_addr,
                               uint8_t *buf, uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_RECV);
  self_ctxt->trace.start();
  socklen_t sock_size = sizeof(out_addr->sockaddr);

  auto res = co_await IORecvFromAwaiter(host_addr.fd, &out_addr->sockaddr,
                                        &sock_size, buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::RECV, res);
}
Task<std::expected<int, ErrorWrapper>>
ESPWIFIUDPTransport::send_to(IOAddress host_addr, IOAddress out_addr,
                             const uint8_t *buf, uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_SEND);
  self_ctxt->trace.start();
  auto res =
      co_await IOSendToAwaiter(host_addr.fd, out_addr.sockaddr, buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::SEND, res);
}

Task<std::expected<int, ErrorWrapper>>
ESPWIFIUDPTransport::io_close(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_UDP_CLOSE);
  self_ctxt->trace.start();

  auto res = close(addr.fd);
  if (res < 0) {
    co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::CLOSE,
                             res);
  }

  auto res2 = wifi_udp_tracker.close_fd(addr.fd);
  if (!res2.has_value()) {
    co_return std::unexpected(res2.error());
  }
  co_return process_io_res(self_ctxt, IOMethod::IO_WIFI_UDP, IOType::CLOSE,
                           res);
}
