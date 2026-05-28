
#include "esp32/io/io.h"
#include "esp_now.h"
void busy_polling::prep_espnow_sendto(const uint8_t *buf, uint64_t buf_size,
                                      const void *send_addr,
                                      const void *user_data,
                                      IOPackageType package_type,
                                      bool with_ack) {

  IOSqe sqe = {};
  sqe.method = IOMethod::IO_ESP_NOW;
  sqe.io_type = IOType::SEND;
  sqe.user_data = user_data;
  sqe.payload.write_payload.buf = buf;
  sqe.payload.write_payload.addr = send_addr;
  sqe.payload.write_payload.buf_size = buf_size;
  sqe.with_ack = with_ack;
  sqe.io_package_type = package_type;
  IOAddress io_addr = {};
  io_addr.addr_type = IOAddressType::MAC;
  memcpy(io_addr.val.sockaddr, send_addr, ESP_NOW_ETH_ALEN);
  IOConnection *connection =
      program_ctxt->connection_handler->get_or_add_connection(&io_addr);
  espnow_ensure_peer((uint8_t *)send_addr);

  auto res2 = connection->write_queue.enque(std::move(sqe));
  if (!res2.has_value()) {
    safe_shutdown(res2.error());
  }
}
void busy_polling::prep_espnow_recvfrom(uint8_t *buf, uint64_t buf_size,
                                        void *recv_addr,
                                        const void *user_data) {
  IOSqe sqe = {};
  sqe.method = IOMethod::IO_ESP_NOW;
  sqe.io_type = IOType::RECV;
  sqe.user_data = user_data;
  sqe.payload.read_payload.buf = buf;
  sqe.payload.read_payload.buf_size = buf_size;
  sqe.payload.read_payload.addr = recv_addr;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
