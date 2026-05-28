
#ifndef WIFI_UDP_PLLING_H
#define WIFI_UDP_PLLING_H
#include "freertos/idf_additions.h"
#include "general/interfaces/io/io.h"
#include "hal/uart_types.h"
#include "lwip/sockets.h"
#include <cstdint>
#include <sys/socket.h>
namespace esp_io {
void prep_sqe_wifi_udp_recv_from(int host_fd, sockaddr_in *recv_socket,
                                 uint8_t *buf, uint64_t buf_size,
                                 const void *data);

void prep_sqe_wifi_udp_send_to(int host_fd, const sockaddr_in *send_socket,
                               const uint8_t *buf, uint64_t buf_size,
                               const void *data, IOPackageType package_type,
                               bool with_ack);

} // namespace esp_io

#endif
