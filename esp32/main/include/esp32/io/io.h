#ifndef ESP_IO_H
#define ESP_IO_H
#include "esp_now.h"
#include "freertos/idf_additions.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/polling_io.h"
#include "general/interfaces/io/transports/udp_transport.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "hal/uart_types.h"
#include "lwip/sockets.h"
#include <cstdint>
class ESPBusyPollingIO : public BusyPollingIO {
public:
  ESPBusyPollingIO(AllocatorInterface *transport_allocator);
  IOTransport *register_transport(IOMethod io_method, uint64_t transport_size);
  template <typename T> T *get_transport(IOMethod io_method);
  void process_all(uint64_t timeout) override;
};

template <typename T> T *ESPBusyPollingIO::get_transport(IOMethod io_method) {
  T *transport_ptr = (T *)this->transports[io_method].transport_ptr.value();
  return transport_ptr;
}
struct ESPNOWSQEConfig : public IOSQEConfig {
  int write_function(IOSqe *sqe, IOConnection *connection,
                     uint8_t *out_buf) override;
  int read_function(IOSqe *sqe) override;
  int open_function(IOSqe *sqe) override;
  int close_function(IOSqe *sqe) override;
  int accept_function(IOSqe *sqe) override;
  int connect_function(IOSqe *sqe) override;
};

#define ESP_NOW_RX_QUEUE_SIZE 32
enum ESPNOW_PACKET_STATE {
  ESPNOW_PACKET_NONE,
  ESPNOW_PACKET_IN_FLIGHT,
  ESPNOW_PACKET_READY,
};
inline ESPNOW_PACKET_STATE esp_now_packet_in_flight = ESPNOW_PACKET_NONE;
inline esp_now_send_status_t esp_now_send_status = ESP_NOW_SEND_SUCCESS;
inline QueueHandle_t esp_now_rx_queue;
struct ESPNOWRxPacket {
  uint64_t len = 0;
  uint8_t data[64] = {0};
  uint8_t mac[ESP_NOW_ETH_ALEN] = {0};
};
namespace busy_polling {
void prep_espnow_sendto(const uint8_t *buf, uint64_t buf_size,
                        const void *send_addr, const void *user_data,
                        IOPackageType package_type, bool with_ack);
void prep_espnow_recvfrom(uint8_t *buf, uint64_t buf_size, void *recv_addr,
                          const void *user_data);

} // namespace busy_polling

#define CONFIG_ESPNOW_CHANNEL 1

bool espnow_ensure_peer(const uint8_t *mac);
class ESPNOWIOTransport : public UDPIOTransport {
public:
  Task<std::expected<int, ErrorWrapper>>
  initialize(const IOAddress *socket_addr, uint64_t listen_backlog) override;

  Task<std::expected<int, ErrorWrapper>> recv_from(const IOAddress *host_addr,
                                                   IOAddress *out_addr,
                                                   uint8_t *buf,
                                                   uint64_t buf_size) override;
  Task<std::expected<int, ErrorWrapper>>
  send_to(const IOAddress *host_addr, const IOAddress *out_addr,
          const uint8_t *buf, uint64_t buf_size,
          IOPackageType package_type) override;

  Task<std::expected<int, ErrorWrapper>>
  send_to(const IOAddress *host_addr, const IOAddress *out_addr,
          const uint8_t *buf, uint64_t buf_size, IOPackageType package_type,
          bool with_ack) override;

  Task<std::expected<int, ErrorWrapper>>
  io_close(const IOAddress *addr, const IOAddress *connection_addr) override;
};
Job espnow_receiver_routine();

#endif
