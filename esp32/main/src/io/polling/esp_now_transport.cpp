
#include "esp32/io/io.h"
#include "esp_now.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/polling_io.h"
#include "general/interfaces/io/transports/busy_polling_awaiters.h"
#include "general/misc/names.h"
class ESPIOSendToAwaiter : public BasicIOAwaiterInterface {
private:
  const uint8_t *buf;
  uint64_t buffer_size;
  const void *send_addr;
  IOPackageType package_type;
  bool with_ack;

public:
  ESPIOSendToAwaiter(const void *send_addr, const uint8_t *buf,
                     uint64_t buffer_size, IOPackageType package_type,
                     bool with_ack) {
    this->package_type = package_type;
    this->send_addr = send_addr;
    this->buf = buf;
    this->buffer_size = buffer_size;
    this->type = IOType::SEND;
    this->io_method = IOMethod::IO_ESP_NOW;
    this->with_ack = with_ack;
  }

  void submit() override {
    busy_polling::prep_espnow_sendto(this->buf, this->buffer_size,
                                     this->send_addr, this, this->package_type,
                                     this->with_ack);
  }
};

class ESPNOWRecvFromAwaiter : public BasicIOAwaiterInterface {
private:
  uint8_t *buf;
  uint64_t buffer_size;
  void *recv_addr;

public:
  ESPNOWRecvFromAwaiter(void *recv_addr, uint8_t *buf, uint64_t buffer_size) {

    this->recv_addr = recv_addr;
    this->buf = buf;
    this->buffer_size = buffer_size;
    this->type = IOType::RECV;
    this->io_method = IOMethod::IO_ESP_NOW;
  }
  void submit() override {
    busy_polling::prep_espnow_recvfrom(this->buf, this->buffer_size,
                                       this->recv_addr, this);
  }
};
Task<std::expected<int, ErrorWrapper>>
ESPNOWIOTransport::initialize(const IOAddress *host_addr,
                              uint64_t listen_backlog) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_ESPNOW_BIND);
  self_ctxt->trace.start();

  co_return 0;
}

Task<std::expected<int, ErrorWrapper>> ESPNOWIOTransport::send_to(
    const IOAddress *host_addr, const IOAddress *out_addr, const uint8_t *buf,
    uint64_t buf_size, IOPackageType package_type, bool with_ack) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_ESPNOW_SEND);
  self_ctxt->trace.start();

  int res = co_await ESPIOSendToAwaiter(out_addr->val.sockaddr, buf, buf_size,
                                        package_type, with_ack);

  co_return process_io_res(self_ctxt, IOMethod::IO_ESP_NOW, IOType::SEND, res);
}

Task<std::expected<int, ErrorWrapper>>
ESPNOWIOTransport::send_to(const IOAddress *host_addr,
                           const IOAddress *out_addr, const uint8_t *buf,
                           uint64_t buf_size, IOPackageType package_type) {
  return this->send_to(host_addr, out_addr, buf, buf_size, package_type, false);
}

Task<std::expected<int, ErrorWrapper>>
ESPNOWIOTransport::recv_from(const IOAddress *host_addr, IOAddress *cli_addr,
                             uint8_t *buf, uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_ESPNOW_RECV);
  self_ctxt->trace.start();

  int res =
      co_await ESPNOWRecvFromAwaiter(cli_addr->val.sockaddr, buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_ESP_NOW, IOType::RECV, res);
}

Task<std::expected<int, ErrorWrapper>>
ESPNOWIOTransport::io_close(const IOAddress *host_addr,
                            const IOAddress *conn_addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_WIFI_ESPNOW_CLOSE);
  self_ctxt->trace.start();
  int res = esp_now_del_peer(host_addr->val.sockaddr);

  co_return process_io_res(self_ctxt, IOMethod::IO_ESP_NOW, IOType::RECV, res);
}
