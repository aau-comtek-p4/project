
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/busy_pollings_transports.h"
#include <cstdint>
#include <cstring>

void busy_polling::prep_tcp_close(int host_fd, const void *user_data) {
  IOSqe sqe = {};
  sqe.method = IOMethod::IO_WIFI_TCP;
  sqe.io_type = IOType::CLOSE;
  sqe.user_data = user_data;
  sqe.payload.close_payload.fd = host_fd;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
void busy_polling::prep_tcp_send(int client_fd, const uint8_t *buf,
                                 uint64_t buf_size, const void *user_data,
                                 IOPackageType package_type, bool with_ack) {
  IOSqe sqe = {};
  sqe.method = IOMethod::IO_WIFI_TCP;
  sqe.io_type = IOType::SEND;
  sqe.user_data = user_data;
  sqe.payload.write_payload.fd = client_fd;
  sqe.payload.write_payload.buf = buf;
  sqe.payload.write_payload.buf_size = buf_size;
  sqe.with_ack = with_ack;
  sqe.io_package_type = package_type;
  IOAddress io_addr = {};
  io_addr.addr_type = IOAddressType::FILE_DESCRIPTOR;
  io_addr.val.fd = client_fd;
  auto res = program_ctxt->connection_handler->get_connection(&io_addr);
  if (!res.has_value()) {
    safe_shutdown(ErrorWrapper{.error = -1, .tag = ErrorWrapper::CUSTOM});
  }

  auto res2 = res.value()->write_queue.enque(std::move(sqe));
  if (!res2.has_value()) {
    safe_shutdown(res2.error());
  }
}
void busy_polling::prep_tcp_connect(int host_fd, const void *connect_addr,
                                    const void *user_data) {

  IOSqe sqe = {};
  sqe.method = IOMethod::IO_WIFI_TCP;
  sqe.io_type = IOType::CONNECT;
  sqe.user_data = user_data;
  sqe.payload.write_payload.fd = host_fd;
  sqe.payload.write_payload.addr = connect_addr;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
void busy_polling::prep_tcp_accept(int host_fd, void *recv_addr,
                                   const void *user_data) {

  IOSqe sqe = {};
  sqe.method = IOMethod::IO_WIFI_TCP;
  sqe.io_type = IOType::ACCEPT;
  sqe.user_data = user_data;
  sqe.payload.read_payload.fd = host_fd;
  sqe.payload.read_payload.addr = recv_addr;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
void busy_polling::prep_tcp_recv(int client_fd, uint8_t *buf, uint64_t buf_size,
                                 const void *user_data) {

  IOSqe sqe = {};
  sqe.method = IOMethod::IO_WIFI_TCP;
  sqe.io_type = IOType::RECV;
  sqe.user_data = user_data;
  sqe.payload.read_payload.fd = client_fd;
  sqe.payload.read_payload.buf = buf;
  sqe.payload.read_payload.buf_size = buf_size;
  IOAddress io_addr = {};
  io_addr.addr_type = IOAddressType::FILE_DESCRIPTOR;
  io_addr.val.fd = client_fd;
  auto res = program_ctxt->connection_handler->get_connection(&io_addr);
  if (!res.has_value()) {
    safe_shutdown(ErrorWrapper{.error = -1, .tag = ErrorWrapper::CUSTOM});
  }
  res.value()->read_sqe = sqe;
}
void busy_polling::prep_tcp_open(const void *host_addr, uint64_t listen_size,
                                 IOTCPType tcp_type, const void *user_data) {
  IOSqe sqe = {};
  sqe.io_type = IOType::OPEN;
  sqe.method = IOMethod::IO_WIFI_TCP;
  sqe.user_data = user_data;
  sqe.payload.socket_open.addr = host_addr;
  sqe.payload.socket_open.listen_size = listen_size;
  sqe.payload.socket_open.tcp_type = tcp_type;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}

void busy_polling::prep_udp_close(int host_fd, const void *conn_addr,
                                  const void *user_data) {
  IOSqe sqe = {};
  sqe.method = IOMethod::IO_WIFI_UDP;
  sqe.io_type = IOType::CLOSE;
  sqe.user_data = user_data;
  sqe.payload.close_payload.fd = host_fd;
  sqe.payload.close_payload.conn_addr = conn_addr;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
void busy_polling::prep_udp_sendto(int client_fd, const uint8_t *buf,
                                   uint64_t buf_size, const void *send_addr,
                                   const void *user_data,
                                   IOPackageType package_type, bool with_ack) {
  IOSqe sqe = {};
  sqe.method = IOMethod::IO_WIFI_UDP;
  sqe.io_type = IOType::SEND;
  sqe.user_data = user_data;
  sqe.payload.write_payload.fd = client_fd;
  sqe.payload.write_payload.buf = buf;
  sqe.payload.write_payload.addr = send_addr;
  sqe.payload.write_payload.buf_size = buf_size;
  sqe.with_ack = with_ack;
  sqe.io_package_type = package_type;
  IOAddress io_addr = {};
  io_addr.addr_type = IOAddressType::IO_SOCKADDR;
  memcpy(io_addr.val.sockaddr, send_addr, sizeof(io_addr.val.sockaddr));
  IOConnection *connection =
      program_ctxt->connection_handler->get_or_add_connection(&io_addr);

  auto res2 = connection->write_queue.enque(std::move(sqe));
  if (!res2.has_value()) {
    safe_shutdown(res2.error());
  }
}

void busy_polling::prep_udp_recvfrom(int client_fd, uint8_t *buf,
                                     uint64_t buf_size, void *recv_addr,
                                     const void *user_data) {
  IOSqe sqe = {};
  sqe.method = IOMethod::IO_WIFI_UDP;
  sqe.io_type = IOType::RECV;
  sqe.user_data = user_data;
  sqe.payload.read_payload.fd = client_fd;
  sqe.payload.read_payload.buf = buf;
  sqe.payload.read_payload.buf_size = buf_size;
  sqe.payload.read_payload.addr = recv_addr;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
void busy_polling::prep_udp_open(const void *host_addr, uint64_t listen_size,
                                 const void *user_data) {
  IOSqe sqe = {};
  sqe.io_type = IOType::OPEN;
  sqe.method = IOMethod::IO_WIFI_UDP;
  sqe.user_data = user_data;
  sqe.payload.socket_open.addr = host_addr;
  sqe.payload.socket_open.listen_size = listen_size;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
// FILE
void busy_polling::prep_file_open(const char *file_path, int flag,
                                  uint32_t mode, const void *user_data) {
  IOSqe sqe = {};
  sqe.user_data = user_data;
  sqe.io_type = IOType::OPEN;
  sqe.method = IOMethod::IO_FILE;
  sqe.payload.file_open.file_path = file_path;
  sqe.payload.file_open.flag = flag;
  sqe.payload.file_open.mode = mode;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
void busy_polling::prep_file_write(int file_fd, const uint8_t *buf,
                                   uint64_t buf_size, const void *user_data) {
  IOSqe sqe = {};
  sqe.user_data = user_data;
  sqe.io_type = IOType::WRITE;
  sqe.method = IOMethod::IO_FILE;
  sqe.payload.write_payload.fd = file_fd;
  sqe.payload.write_payload.buf = buf;
  sqe.payload.write_payload.buf_size = buf_size;

  IOAddress io_adress = {};
  io_adress.addr_type = IOAddressType::FILE_DESCRIPTOR;
  io_adress.val.fd = file_fd;
  IOConnection *connection =
      program_ctxt->connection_handler->get_or_add_connection(&io_adress);
  auto res = connection->write_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
void busy_polling::prep_file_read(int file_fd, uint8_t *buf, uint64_t buf_size,
                                  const void *user_data) {
  IOSqe sqe = {};
  sqe.user_data = user_data;
  sqe.io_type = IOType::READ;
  sqe.method = IOMethod::IO_FILE;
  sqe.payload.read_payload.fd = file_fd;
  sqe.payload.read_payload.buf = buf;
  sqe.payload.read_payload.buf_size = buf_size;

  IOAddress io_adress = {};
  io_adress.addr_type = IOAddressType::FILE_DESCRIPTOR;
  io_adress.val.fd = file_fd;
  IOConnection *connection =
      program_ctxt->connection_handler->get_or_add_connection(&io_adress);
  connection->read_sqe = sqe;
}
void busy_polling::prep_file_close(int file_fd, const void *user_data) {
  IOSqe sqe = {};
  sqe.method = IOMethod::IO_FILE;
  sqe.io_type = IOType::CLOSE;
  sqe.user_data = user_data;
  sqe.payload.close_payload.fd = file_fd;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}

void busy_polling::prep_serial_open(const char *file_path,
                                    const void *user_data) {
  IOSqe sqe = {};
  sqe.user_data = user_data;
  sqe.io_type = IOType::OPEN;
  sqe.method = IOMethod::IO_SERIAL;
  sqe.payload.file_open.file_path = file_path;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
void busy_polling::prep_serial_write(int file_fd, const uint8_t *buf,
                                     uint64_t buf_size, const void *user_data,
                                     IOPackageType package_type,
                                     bool with_ack) {
  IOSqe sqe = {};
  sqe.user_data = user_data;
  sqe.io_type = IOType::WRITE;
  sqe.method = IOMethod::IO_SERIAL;
  sqe.payload.write_payload.fd = file_fd;
  sqe.payload.write_payload.buf = buf;
  sqe.payload.write_payload.buf_size = buf_size;
  sqe.with_ack = with_ack;
  sqe.io_package_type = package_type;

  IOAddress io_adress = {};
  io_adress.addr_type = IOAddressType::FILE_DESCRIPTOR;
  io_adress.val.fd = file_fd;
  IOConnection *connection =
      program_ctxt->connection_handler->get_or_add_connection(&io_adress);
  auto res = connection->write_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
void busy_polling::prep_serial_read(int file_fd, uint8_t *buf,
                                    uint64_t buf_size, const void *user_data) {
  IOSqe sqe = {};
  sqe.user_data = user_data;
  sqe.io_type = IOType::READ;
  sqe.method = IOMethod::IO_SERIAL;
  sqe.payload.read_payload.fd = file_fd;
  sqe.payload.read_payload.buf = buf;
  sqe.payload.read_payload.buf_size = buf_size;

  IOAddress io_adress = {};
  io_adress.addr_type = IOAddressType::FILE_DESCRIPTOR;
  io_adress.val.fd = file_fd;
  IOConnection *connection =
      program_ctxt->connection_handler->get_or_add_connection(&io_adress);
  connection->read_sqe = sqe;
}
void busy_polling::prep_serial_close(int file_fd, const void *user_data) {
  IOSqe sqe = {};
  sqe.method = IOMethod::IO_SERIAL;
  sqe.io_type = IOType::CLOSE;
  sqe.user_data = user_data;
  sqe.payload.close_payload.fd = file_fd;

  auto res = program_ctxt->io->sqe_queue.enque(std::move(sqe));
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
}
