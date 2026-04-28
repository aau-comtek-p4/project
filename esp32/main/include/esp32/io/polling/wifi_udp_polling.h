
#ifndef WIFI_UDP_PLLING_H
#define WIFI_UDP_PLLING_H
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"
#include "lwip/sockets.h"
#include <cstdint>
#include <sys/socket.h>
void poll_wifi_udp();
namespace esp_io {
void prep_sqe_wifi_udp_recv_from(int host_fd, sockaddr_in *recv_socket,
                                 socklen_t *sock_size, uint8_t *buf,
                                 uint64_t buf_size, const void *data);

void prep_sqe_wifi_udp_send_to(int host_fd, sockaddr_in send_socket,
                               const uint8_t *buf, uint64_t buf_size,
                               const void *data);

} // namespace esp_io

#endif
