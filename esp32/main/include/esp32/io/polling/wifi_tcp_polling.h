
#ifndef WIFI_TCP_PLLING_H
#define WIFI_TCP_PLLING_H
#include "freertos/idf_additions.h"
#include "general/interfaces/io/io.h"
#include "hal/uart_types.h"
#include "lwip/sockets.h"
#include <cstdint>
#include <sys/socket.h>
namespace esp_io {

void prep_sqe_wifi_tcp_accpt(int host_fd, sockaddr_in *cli_addr,
                             const void *data);
void prep_sqe_wifi_tcp_connect(int host_fd, const sockaddr_in *cli_addr,
                               const void *data);
void prep_sqe_wifi_tcp_recv(int client_fd, uint8_t *buf, uint64_t buf_size,
                            const void *data);

void prep_sqe_wifi_tcp_send(int client_fd, const uint8_t *buf,
                            uint64_t buf_size, const void *data,
                            IOPackageType package_type, bool with_ack);
} // namespace esp_io

#endif
