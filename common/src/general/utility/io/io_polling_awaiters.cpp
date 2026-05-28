
#include "general/common.h"
#include "general/interfaces/io/io.h"

#include "general/interfaces/io/polling_io.h"
#include "general/interfaces/io/transports/busy_polling_awaiters.h"
#include "general/interfaces/io/transports/busy_pollings_transports.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/shutdown.h"
#include <cstdint>
#include <cstdio>
#include <sys/types.h>
busy_polling_awaiters::IOSendAwaiter::IOSendAwaiter(int fd, const uint8_t *buf,
                                                    uint64_t buffer_size,
                                                    IOPackageType package_type,
                                                    bool with_ack) {
  this->fd = fd;
  this->type = IOType::SEND;
  this->buf = buf;
  this->buffer_size = buffer_size;
  this->io_method = IOMethod::IO_WIFI_TCP;
  this->package_type = package_type;
  this->with_ack = with_ack;
}
void busy_polling_awaiters::IOSendAwaiter::submit() {
  busy_polling::prep_tcp_send(this->fd, this->buf, this->buffer_size, this,
                              this->package_type, this->with_ack);
}
busy_polling_awaiters::IORecvAwaiter::IORecvAwaiter(int fd, uint8_t *buf,
                                                    uint64_t buffer_size) {
  this->fd = fd;
  this->buf = buf;
  this->buffer_size = buffer_size;
  this->type = IOType::RECV;
  this->io_method = IOMethod::IO_WIFI_TCP;
}
void busy_polling_awaiters::IORecvAwaiter::submit() {
  busy_polling::prep_tcp_recv(this->fd, this->buf, this->buffer_size, this);
}

busy_polling_awaiters::IOConnectAwaiter::IOConnectAwaiter(
    int fd, const void *sock_addr) {
  this->fd = fd;
  this->type = IOType::CONNECT;
  this->io_method = IOMethod::IO_WIFI_TCP;
  this->sock_addr = sock_addr;
}
void busy_polling_awaiters::IOConnectAwaiter::submit() {
  busy_polling::prep_tcp_connect(this->fd, this->sock_addr, this);
}

busy_polling_awaiters::IOAcceptAwaiter::IOAcceptAwaiter(int fd,
                                                        void *sock_addr) {
  this->fd = fd;
  this->type = IOType::ACCEPT;
  this->io_method = IOMethod::IO_WIFI_TCP;
  this->sock_addr = sock_addr;
}
void busy_polling_awaiters::IOAcceptAwaiter::submit() {
  busy_polling::prep_tcp_accept(this->fd, this->sock_addr, this);
}
busy_polling_awaiters::IOSendToAwaiter::IOSendToAwaiter(
    int fd, const void *send_addr, const uint8_t *buf, uint64_t buffer_size,
    IOPackageType package_type, bool with_ack) {
  this->package_type = package_type;
  this->fd = fd;
  this->send_addr = send_addr;
  this->buf = buf;
  this->buffer_size = buffer_size;
  this->type = IOType::SEND;
  this->io_method = IOMethod::IO_WIFI_UDP;
  this->with_ack = with_ack;
}
void busy_polling_awaiters::IOSendToAwaiter::submit() {
  busy_polling::prep_udp_sendto(this->fd, this->buf, this->buffer_size,
                                this->send_addr, this, this->package_type,
                                this->with_ack);
}

busy_polling_awaiters::IORecvFromAwaiter::IORecvFromAwaiter(
    int fd, void *recv_addr, uint8_t *buf, uint64_t buffer_size) {

  this->fd = fd;
  this->recv_addr = recv_addr;
  this->buf = buf;
  this->buffer_size = buffer_size;
  this->type = IOType::RECV;
  this->io_method = IOMethod::IO_WIFI_UDP;
}
void busy_polling_awaiters::IORecvFromAwaiter::submit() {
  busy_polling::prep_udp_recvfrom(this->fd, this->buf, this->buffer_size,
                                  this->recv_addr, this);
}

busy_polling_awaiters::IOFileOpenAwaiter::IOFileOpenAwaiter(
    const char *file_path, int flag, uint32_t mode) {

  this->type = IOType::OPEN;
  this->io_method = IOMethod::IO_FILE;
  this->file_path = file_path;
  this->flag = flag;
  this->mode = mode;
}
void busy_polling_awaiters::IOFileOpenAwaiter::submit() {
  busy_polling::prep_file_open(this->file_path, this->flag, this->mode, this);
}

busy_polling_awaiters::IOSerialOpenAwaiter::IOSerialOpenAwaiter(
    const char *file_path) {
  this->type = IOType::OPEN;
  this->io_method = IOMethod::IO_SERIAL;
  this->file_path = file_path;
}
void busy_polling_awaiters::IOSerialOpenAwaiter::submit() {
  busy_polling::prep_serial_open(this->file_path, this);
}

busy_polling_awaiters::IOUDPOpenAwaiter::IOUDPOpenAwaiter(
    const void *addr, uint64_t listen_size) {

  this->type = IOType::OPEN;
  this->io_method = IOMethod::IO_WIFI_UDP;
  this->listen_size = listen_size;
  this->addr = addr;
}
void busy_polling_awaiters::IOUDPOpenAwaiter::submit() {
  busy_polling::prep_udp_open(this->addr, this->listen_size, this);
}

busy_polling_awaiters::IOTCPOpenAwaiter::IOTCPOpenAwaiter(const void *addr,
                                                          uint64_t listen_size,
                                                          IOTCPType tcp_type) {

  this->type = IOType::OPEN;
  this->io_method = IOMethod::IO_WIFI_TCP;
  this->listen_size = listen_size;
  this->addr = addr;
  this->tcp_type = tcp_type;
}
void busy_polling_awaiters::IOTCPOpenAwaiter::submit() {
  busy_polling::prep_tcp_open(this->addr, this->listen_size, this->tcp_type,
                              this);
}

busy_polling_awaiters::IOTCPCloseAwaiter::IOTCPCloseAwaiter(int fd) {
  this->type = IOType::CLOSE;
  this->io_method = IOMethod::IO_WIFI_TCP;
  this->fd = fd;
}

void busy_polling_awaiters::IOTCPCloseAwaiter::submit() {
  busy_polling::prep_tcp_close(this->fd, this);
}

busy_polling_awaiters::IOUDPCloseAwaiter::IOUDPCloseAwaiter(
    int fd, const void *conn_addr) {
  this->type = IOType::CLOSE;
  this->io_method = IOMethod::IO_WIFI_UDP;
  this->fd = fd;
  this->conn_addr = conn_addr;
}

void busy_polling_awaiters::IOUDPCloseAwaiter::submit() {
  busy_polling::prep_udp_close(this->fd, this->conn_addr, this);
}

busy_polling_awaiters::IOFileCloseAwaiter::IOFileCloseAwaiter(int fd) {
  this->type = IOType::CLOSE;
  this->io_method = IOMethod::IO_FILE;
  this->fd = fd;
}

void busy_polling_awaiters::IOFileCloseAwaiter::submit() {
  busy_polling::prep_file_close(this->fd, this);
}

busy_polling_awaiters::IOSerialCloseAwaiter::IOSerialCloseAwaiter(int fd) {
  this->type = IOType::CLOSE;
  this->io_method = IOMethod::IO_SERIAL;
  this->fd = fd;
}

void busy_polling_awaiters::IOSerialCloseAwaiter::submit() {
  busy_polling::prep_serial_close(this->fd, this);
}

busy_polling_awaiters::IOFileWriteAwaiter::IOFileWriteAwaiter(
    int fd, const uint8_t *buf, uint64_t buf_size) {
  this->type = IOType::WRITE;
  this->io_method = IOMethod::IO_FILE;
  this->fd = fd;
  this->buf = buf;
  this->buf = buf;
  this->buf_size = buf_size;
}

void busy_polling_awaiters::IOFileWriteAwaiter::submit() {
  busy_polling::prep_file_write(this->fd, this->buf, this->buf_size, this);
}

busy_polling_awaiters::IOSerialWriteAwaiter::IOSerialWriteAwaiter(
    int fd, const uint8_t *buf, uint64_t buf_size, IOPackageType package_type,
    bool with_ack) {
  this->type = IOType::WRITE;
  this->io_method = IOMethod::IO_SERIAL;
  this->fd = fd;
  this->buf = buf;
  this->buf = buf;
  this->buf_size = buf_size;
  this->package_type = package_type;
  this->with_ack = with_ack;
}

void busy_polling_awaiters::IOSerialWriteAwaiter::submit() {
  busy_polling::prep_serial_write(this->fd, this->buf, this->buf_size, this,
                                  this->package_type, this->with_ack);
}

busy_polling_awaiters::IOFileReadAwaiter::IOFileReadAwaiter(int fd,
                                                            uint8_t *buf,
                                                            uint64_t buf_size) {
  this->type = IOType::READ;
  this->io_method = IOMethod::IO_FILE;
  this->fd = fd;
  this->buf = buf;
  this->buf = buf;
  this->buf_size = buf_size;
}
void busy_polling_awaiters::IOFileReadAwaiter::submit() {
  busy_polling::prep_file_read(this->fd, this->buf, this->buf_size, this);
}

busy_polling_awaiters::IOSerialReadAwaiter::IOSerialReadAwaiter(
    int fd, uint8_t *buf, uint64_t buf_size) {
  this->type = IOType::READ;
  this->io_method = IOMethod::IO_SERIAL;
  this->fd = fd;
  this->buf = buf;
  this->buf = buf;
  this->buf_size = buf_size;
}
void busy_polling_awaiters::IOSerialReadAwaiter::submit() {
  busy_polling::prep_serial_read(this->fd, this->buf, this->buf_size, this);
}
