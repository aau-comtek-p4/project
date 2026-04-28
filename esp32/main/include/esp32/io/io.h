#ifndef ESP_IO_H
#define ESP_IO_H
#include "freertos/idf_additions.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "hal/uart_types.h"
#include "lwip/sockets.h"
#include <cstdint>
struct IOAddress {
  enum { UART_PORT, IO_SOCKADDR, FILE_DESCRIPTOR } addr_type;
  union {
    uart_port_t uart_port;
    sockaddr_in sockaddr;
    int fd;
  };
};
class ESPIOAwaiterInterface : public IOAwaitInterface {
public:
  int result;
  const char *get_type();
  virtual void submit() = 0;
  bool await_ready();
  bool await_suspend(std::coroutine_handle<
                     Task<std::expected<int, ErrorWrapper>>::promise_type>
                         handle);
  int await_resume();
};

class ESPIOHandler : public IOHandler {
private:
  AllocatorInterface *transport_allocator;
  IOTransportWrapper transports[IOMethod::IO_END];

public:
  ESPIOHandler(AllocatorInterface *transport_allocator);
  IOTransport *register_transport(IOMethod io_method, uint64_t transport_size);
  template <typename T> T *get_transport(IOMethod io_method);
  void submit_all();
  void process_all(uint64_t timeout) override;
  void cancel(IOMethod io_method, const void *user_data);
};

template <typename T> T *ESPIOHandler::get_transport(IOMethod io_method) {
  T *transport_ptr = (T *)this->transports[io_method].transport_ptr.value();
  return transport_ptr;
}
struct ESPIOSqePayload {
  union {
    struct {
      uint8_t *buf;
      uint64_t buf_size;
    } serial_uart_read;
    struct {
      const uint8_t *buf;
      uint64_t buf_size;
    } serial_uart_write;
    struct {
      sockaddr_in *recv_addr;
      socklen_t *addr_size;
      uint8_t *buf;
      uint64_t buf_size;
    } wifi_udp_recv;
    struct {
      sockaddr_in send_addr;
      const uint8_t *buf;
      uint64_t buf_size;
    } wifi_udp_send;
  };
};
struct ESPIOSqe {
  ESPIOSqePayload payload;
  const void *user_data;
};
struct ESPIOCqe {
  int32_t res;
  const void *user_data;
};
struct FDHolder {
  uint8_t fd;
  bool active;
};

#define SQE_MAX_QUEUES_UART 3
#define SQE_MAX_QUEUES_FD 7

struct FDTracker {

public:
  QueueHandle_t fd_queue[SQE_MAX_QUEUES_FD];
  FDHolder holders[SQE_MAX_QUEUES_FD];
  std::expected<void, ErrorWrapper> register_fd(int fd);
  std::expected<QueueHandle_t, ErrorWrapper> get_queue(int fd);
  std::expected<void, ErrorWrapper> close_fd(int fd);
};

#define SQE_MAX_QUEUES_PEERS 7
#define SQE_MAX_QUEUES_TOTAL                                                   \
  (SQE_MAX_QUEUES_UART + SQE_MAX_QUEUES_FD * 2 + SQE_MAX_QUEUES_PEERS)

#define SQE_UART_OFFSET 0
#define SQE_FD_TCP_OFFSET SQE_MAX_QUEUES_UART
#define SQE_FD_UDP_OFFSET SQE_MAX_QUEUES_UART + SQE_MAX_QUEUES_FD
#define SQE_PEERS_OFFSET SQE_MAX_QUEUES_UART + SQE_MAX_QUEUES_FD * 2

inline QueueHandle_t sqe_no_retry;
inline QueueHandle_t sqe_uart_queue[SQE_MAX_QUEUES_UART];

inline FDTracker wifi_udp_tracker;
inline FDTracker wifi_tcp_tracker;

inline QueueHandle_t sqe_esp_now_queue[SQE_MAX_QUEUES_PEERS];
inline QueueHandle_t cqe_queue;

#endif
