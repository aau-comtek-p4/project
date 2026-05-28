
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/polling_io.h"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>
#include <termios.h>
#include <unistd.h>
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
    return -errno;
  }
  int fcntl_res = fcntl(listen_sock, F_SETFL, O_NONBLOCK);
  if (fcntl_res < 0) {
    return -errno;
  }
  if (sqe->payload.socket_open.tcp_type == IOTCPType::TCP_CLIENT) {
    return listen_sock;
  }
  int bind_res =
      bind(listen_sock, (const sockaddr *)sqe->payload.socket_open.addr,
           sizeof(sockaddr_in));
  if (bind_res < 0) {
    return -errno;
  }
  int listen_res = listen(listen_sock, sqe->payload.socket_open.listen_size);
  if (listen_res < 0) {
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

  program_ctxt->logger->log_entry(logging::log_debug("UDP read"));
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
    return -errno;
  }

  int fcntl_res = fcntl(listen_sock, F_SETFL, O_NONBLOCK);
  if (fcntl_res < 0) {
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
  int res = write(sqe->payload.write_payload.fd, out_buf, bytes_written);
  if (res < 0) {
    res = -errno;
  }

  return res;
}
int SerialSQEConfig::read_function(IOSqe *sqe) {
  int res = read(sqe->payload.read_payload.fd, sqe->payload.read_payload.buf,
                 sqe->payload.read_payload.buf_size);
  if (res < 0) {
    res = -errno;
  }
  if (res == 0) {
    res = -EAGAIN;
  }
  return res;
}
int SerialSQEConfig::open_function(IOSqe *sqe) {
  int fd =
      open(sqe->payload.file_open.file_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    return -errno;
  }

  termios tty{};
  if (tcgetattr(fd, &tty) < 0) {
    close(fd);
    return -errno;
  }

  cfsetispeed(&tty, B921600);
  cfsetospeed(&tty, B921600);

  // Raw mode
  cfmakeraw(&tty);

  // 8N1
  tty.c_cflag &= ~PARENB; // no parity
  tty.c_cflag &= ~CSTOPB; // 1 stop bit
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8; // 8 data bits

  // Disable flow control
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cflag |= CREAD | CLOCAL;

  // Non-blocking read
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 0;

  if (tcsetattr(fd, TCSANOW, &tty) < 0) {
    close(fd);
    return -errno;
  }

  return fd;
  return 0;
}
int SerialSQEConfig::close_function(IOSqe *sqe) {
  IOAddress io_address;
  io_address.addr_type = IOAddressType::FILE_DESCRIPTOR;
  io_address.val.fd = sqe->payload.close_payload.fd;

  program_ctxt->connection_handler->close_connection(&io_address);
  int res = close(sqe->payload.close_payload.fd);
  if (res < 0) {
    res = -errno;
  }

  return res;
}
int SerialSQEConfig::accept_function(IOSqe *sqe) { return 0; }
int SerialSQEConfig::connect_function(IOSqe *sqe) { return 0; }
int FileSQEConfig::write_function(IOSqe *sqe, IOConnection *connection,
                                  uint8_t *out_buf) {
  int res = write(sqe->payload.write_payload.fd, sqe->payload.write_payload.buf,
                  sqe->payload.write_payload.buf_size);
  if (res < 0) {
    res = -errno;
  }
  return res;
};
int FileSQEConfig::read_function(IOSqe *sqe) {
  int res = read(sqe->payload.read_payload.fd, sqe->payload.read_payload.buf,
                 sqe->payload.read_payload.buf_size);
  if (res < 0) {
    res = -errno;
  }
  return res;
};
int FileSQEConfig::open_function(IOSqe *sqe) {
  int res = open(sqe->payload.file_open.file_path, sqe->payload.file_open.flag,
                 sqe->payload.file_open.mode);
  if (res < 0) {
    res = -errno;
  }
  return res;
};
int FileSQEConfig::close_function(IOSqe *sqe) {
  IOAddress io_address;
  io_address.addr_type = IOAddressType::FILE_DESCRIPTOR;
  io_address.val.fd = sqe->payload.close_payload.fd;
  program_ctxt->connection_handler->close_connection(&io_address);
  int res = close(sqe->payload.close_payload.fd);
  if (res < 0) {
    res = -errno;
  }
  return res;
};
int FileSQEConfig::accept_function(IOSqe *sqe) { return 0; };
int FileSQEConfig::connect_function(IOSqe *sqe) { return 0; };
