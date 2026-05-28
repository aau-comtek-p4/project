#include "driver/uart.h"
#include "esp32/io/io.h"
#include "esp_now.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/polling_io.h"
#include "general/interfaces/utility/logger.h"
#include "hal/uart_types.h"
#include "lwip/sockets.h"
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/_default_fcntl.h>
#include <sys/unistd.h>
int TCPSQEConfig::write_function(IOSqe *sqe, IOConnection *connection,
                                 uint8_t *out_buf) {
  uint64_t bytes_written = parse_sqe_write(sqe, connection, out_buf);
  int res = send(sqe->payload.write_payload.fd, out_buf, bytes_written, 0);
  if (res < 0) {
    res = -errno;
  }
  return res;
}
int TCPSQEConfig::read_function(IOSqe *sqe) {

  int res = recv(sqe->payload.read_payload.fd, sqe->payload.read_payload.buf,
                 sqe->payload.read_payload.buf_size, 0);
  if (res < 0) {
    res = -errno;
  }

  return res;
}
int TCPSQEConfig::open_function(IOSqe *sqe) {
  int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sock < 0) {
    close(listen_sock);
    return -errno;
  }
  int fcntl_res = fcntl(listen_sock, F_SETFL, O_NONBLOCK);
  if (fcntl_res < 0) {
    close(listen_sock);
    return -errno;
  }
  if (sqe->payload.socket_open.tcp_type == IOTCPType::TCP_CLIENT) {
    return listen_sock;
  }
  int bind_res =
      bind(listen_sock, (const sockaddr *)sqe->payload.socket_open.addr,
           sizeof(sockaddr_in));
  if (bind_res < 0) {
    close(listen_sock);
    return -errno;
  }
  int listen_res = listen(listen_sock, sqe->payload.socket_open.listen_size);
  if (listen_res < 0) {
    close(listen_sock);
    return -errno;
  }
  return listen_sock;
}
int TCPSQEConfig::close_function(IOSqe *sqe) {
  int res = close(sqe->payload.close_payload.fd);
  if (res < 0) {
    return -errno;
  }
  IOAddress io_address = {};
  io_address.addr_type = IOAddressType::FILE_DESCRIPTOR;
  io_address.val.fd = sqe->payload.close_payload.fd;
  program_ctxt->connection_handler->close_connection(&io_address);

  return res;
}
int TCPSQEConfig::accept_function(IOSqe *sqe) {
  IOAddress io_address = {};
  io_address.addr_type = IOAddressType::FILE_DESCRIPTOR;
  socklen_t sock_len = sizeof(sockaddr_in);
  int res = accept(sqe->payload.read_payload.fd,
                   (sockaddr *)sqe->payload.read_payload.addr, &sock_len);
  if (res < 0) {
    res = -errno;
  } else {

    io_address.val.fd = res;
    program_ctxt->connection_handler->add_connection(&io_address);
  }
  return res;
}
int TCPSQEConfig::connect_function(IOSqe *sqe) {
  IOAddress io_address = {};
  io_address.addr_type = IOAddressType::FILE_DESCRIPTOR;
  int res = connect(sqe->payload.write_payload.fd,
                    (const sockaddr *)sqe->payload.write_payload.addr,
                    sizeof(sockaddr_in));
  if (res < 0) {
    res = -errno;
  } else {
    io_address.val.fd = res;
    program_ctxt->connection_handler->add_connection(&io_address);
  }
  return res;
}

int UDPSQEConfig::write_function(IOSqe *sqe, IOConnection *connection,
                                 uint8_t *out_buf) {
  uint64_t bytes_written = parse_sqe_write(sqe, connection, out_buf);
  int res = sendto(sqe->payload.write_payload.fd, out_buf, bytes_written, 0,
                   (const sockaddr *)sqe->payload.write_payload.addr,
                   sizeof(sockaddr_in));
  if (res < 0) {
    res = -errno;
  }

  return res;
}
int UDPSQEConfig::read_function(IOSqe *sqe) {

  socklen_t sock_len = sizeof(sockaddr_in);
  int res =
      recvfrom(sqe->payload.read_payload.fd, sqe->payload.read_payload.buf,
               sqe->payload.read_payload.buf_size, 0,
               (sockaddr *)sqe->payload.read_payload.addr, &sock_len);
  if (res < 0) {
    res = -errno;
  }

  IOAddress io_address = {};
  io_address.addr_type = IOAddressType::IO_SOCKADDR;
  memcpy(io_address.val.sockaddr, sqe->payload.read_payload.addr,
         sizeof(sockaddr_in));
  program_ctxt->connection_handler->get_or_add_connection(&io_address);
  return res;
}
int UDPSQEConfig::open_function(IOSqe *sqe) {
  int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sock < 0) {
    close(listen_sock);
    return -errno;
  }

  int fcntl_res = fcntl(listen_sock, F_SETFL, O_NONBLOCK);
  if (fcntl_res < 0) {
    close(listen_sock);
    return -errno;
  }
  return listen_sock;
}
int UDPSQEConfig::close_function(IOSqe *sqe) {

  IOAddress io_address = {};
  io_address.addr_type = IOAddressType::IO_SOCKADDR;
  memcpy(io_address.val.sockaddr, sqe->payload.close_payload.conn_addr,
         sizeof(sockaddr_in));
  auto possible_connection =
      program_ctxt->connection_handler->get_connection(&io_address);
  if (possible_connection.has_value()) {
    program_ctxt->connection_handler->close_connection(&io_address);
    return 0;
  }

  int res = close(sqe->payload.close_payload.fd);
  if (res < 0) {
    return -errno;
  }

  return res;
}
int UDPSQEConfig::accept_function(IOSqe *sqe) { return 0; }
int UDPSQEConfig::connect_function(IOSqe *sqe) { return 0; }

int SerialSQEConfig::write_function(IOSqe *sqe, IOConnection *connection,
                                    uint8_t *out_buf) {
  uint64_t bytes_written = parse_uart_write(sqe, connection, out_buf);
  int res = uart_write_bytes((uart_port_t)sqe->payload.write_payload.fd,
                             out_buf, bytes_written);
  if (res < 0) {
    res = -errno;
  }

  return res;
}
int SerialSQEConfig::read_function(IOSqe *sqe) {
  int res = uart_read_bytes((uart_port_t)sqe->payload.read_payload.fd,
                            sqe->payload.read_payload.buf,
                            sqe->payload.read_payload.buf_size, 0);
  if (res < 0) {
    res = -errno;
  }

  if (res == 0) {
    res = -EAGAIN;
  }
  return res;
}
int SerialSQEConfig::open_function(IOSqe *sqe) { return 0; }
int SerialSQEConfig::close_function(IOSqe *sqe) { return 0; }
int SerialSQEConfig::accept_function(IOSqe *sqe) { return 0; }
int SerialSQEConfig::connect_function(IOSqe *sqe) { return 0; }

int ESPNOWSQEConfig::write_function(IOSqe *sqe, IOConnection *connection,
                                    uint8_t *out_buf) {
  if (esp_now_packet_in_flight ==
      ESPNOW_PACKET_STATE::ESPNOW_PACKET_IN_FLIGHT) {
    return -EAGAIN;
  }

  if (esp_now_packet_in_flight == ESPNOW_PACKET_STATE::ESPNOW_PACKET_READY) {
    esp_now_packet_in_flight = ESPNOW_PACKET_STATE::ESPNOW_PACKET_NONE;
    return sqe->payload.write_payload.buf_size;
  }

  uint64_t bytes_written = parse_sqe_write(sqe, connection, out_buf);
  int res = esp_now_send((uint8_t *)sqe->payload.write_payload.addr, out_buf,
                         bytes_written);
  if (res < 0) {
    res = -errno;
  }
  esp_now_packet_in_flight = ESPNOW_PACKET_STATE::ESPNOW_PACKET_IN_FLIGHT;

  return -EAGAIN;
}
int ESPNOWSQEConfig::read_function(IOSqe *sqe) {
  ESPNOWRxPacket rx_packet;
  if (!xQueueReceive(esp_now_rx_queue, &rx_packet, 0)) {
    return -EAGAIN;
  }
  IOAddress io_addr = {};
  io_addr.addr_type = IOAddressType::MAC;
  memcpy(io_addr.val.sockaddr, rx_packet.mac, ESP_NOW_ETH_ALEN);
  memcpy(sqe->payload.read_payload.buf, rx_packet.data, rx_packet.len);
  memcpy(sqe->payload.read_payload.addr, rx_packet.mac, ESP_NOW_ETH_ALEN);
  IOConnection *io_connection =
      program_ctxt->connection_handler->get_or_add_connection(&io_addr);
  espnow_ensure_peer(rx_packet.mac);

  return rx_packet.len;
}

int ESPNOWSQEConfig::open_function(IOSqe *sqe) { return 0; }
int ESPNOWSQEConfig::close_function(IOSqe *sqe) { return 0; }
int ESPNOWSQEConfig::accept_function(IOSqe *sqe) { return 0; }
int ESPNOWSQEConfig::connect_function(IOSqe *sqe) { return 0; }
