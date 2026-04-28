
#include "esp32/io/polling/wifi_udp_polling.h"
#include "esp32/io/io.h"
#include "freertos/idf_additions.h"
#include "general/misc/shutdown.h"
#include "lwip/sockets.h"
#include <sys/socket.h>
void esp_io::prep_sqe_wifi_udp_recv_from(int host_fd, sockaddr_in *recv_socket,
                                         socklen_t *sock_size, uint8_t *buf,
                                         uint64_t buf_size, const void *data) {
  ESPIOSqe sqe;
  sqe.user_data = data;
  sqe.payload.wifi_udp_recv.buf = buf;
  sqe.payload.wifi_udp_recv.buf_size = buf_size;
  sqe.payload.wifi_udp_recv.recv_addr = recv_socket;
  sqe.payload.wifi_udp_recv.addr_size = sock_size;
  auto res = wifi_udp_tracker.get_queue(host_fd);
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
  xQueueSend(res.value(), &sqe, 0);
}
void esp_io::prep_sqe_wifi_udp_send_to(int host_fd, sockaddr_in send_socket,
                                       const uint8_t *buf, uint64_t buf_size,
                                       const void *data) {
  ESPIOSqe sqe;
  sqe.user_data = data;
  sqe.payload.wifi_udp_send.buf = buf;
  sqe.payload.wifi_udp_send.buf_size = buf_size;
  sqe.payload.wifi_udp_send.send_addr = send_socket;
  auto res = wifi_udp_tracker.get_queue(host_fd);
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
  xQueueSend(res.value(), &sqe, 0);
}

void poll_wifi_udp_fd(int fd, QueueHandle_t sqe_queue) {
  ESPIOSqe sqe;
  while (xQueuePeek(sqe_queue, &sqe, 0)) {
    int res = recvfrom(fd, sqe.payload.wifi_udp_recv.buf,
                       sqe.payload.wifi_udp_recv.buf_size, 0,
                       (sockaddr *)sqe.payload.wifi_udp_recv.recv_addr,
                       sqe.payload.wifi_udp_recv.addr_size);
    if (res < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      return;
    }
    xQueueReceive(sqe_queue, &sqe, 0);
    ESPIOCqe cqe{.res = res, .user_data = sqe.user_data};
    xQueueSend(cqe_queue, &cqe, 0);
  }
}

void poll_wifi_udp() {
  for (int i = 0; i < SQE_MAX_QUEUES_FD; i++) {
    if (wifi_udp_tracker.holders[i].active) {
      poll_wifi_udp_fd(wifi_udp_tracker.holders[i].fd,
                       wifi_udp_tracker.fd_queue[i]);
    }
  }
}
