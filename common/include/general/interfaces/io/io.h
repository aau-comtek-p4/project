#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include "general/misc/names.h"
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <sys/types.h>

#define IO_TAG "IO"
#define IO_ERROR_TAG "IO ERROR"
#define UART_MAGIC_HEADER 0xDEADBEEF
#define UART_BAUDRATE 921600
#define CRC32_START 0xFFFFFFFF

#define IO_MAX_CONNECTIONS 20

enum IOTCPType {
  TCP_SERVER,
  TCP_CLIENT,
};
enum IOPackageType : uint8_t {
  IOPackage_NULL = 0,
  IOPackage_LOG = 1,
  IOPackage_ACK = 2,
  IOPackage_PROTECTION_DATA = 3,
  IOPackage_CONNECTION_PACKAGE = 4,
  IOPackage_CONTROL_DATA = 5,
};
enum IODeviceType : uint8_t {
  DEVICE_BASE_STATION,
  DEVICE_CENTRAL_SERVER,
  DEVICE_SENSOR_NODE,
  DEVICE_INTERMEDIATE_SERVER,

};
struct IOPackageHeader {
  uint64_t seq;
  uint8_t optional_data[4] = {};
  IOPackageType package_type;
  uint8_t device_id;
  IODeviceType device_type;
  bool with_ack;
};
struct IOUARTFrameHeader {
  IOPackageHeader package_header;
  uint32_t magic;
  uint8_t optional_data[4] = {};
};

class IOAwaitInterface {
public:
  IOType type;
  IOMethod io_method;
  std::coroutine_handle<> handle;
  virtual const char *get_type() = 0;
  virtual bool await_ready() = 0;
  virtual bool
  await_suspend(std::coroutine_handle<
                Task<std::expected<int, ErrorWrapper>>::promise_type>
                    handle) = 0;
  virtual int await_resume() = 0;
};

class BasicIOAwaiterInterface : public IOAwaitInterface {

public:
  int result = 0;
  const char *get_type();
  virtual void submit() = 0;
  bool await_ready();
  bool await_suspend(std::coroutine_handle<
                     Task<std::expected<int, ErrorWrapper>>::promise_type>
                         handle);
  int await_resume();
};

class IOTransport {};

struct IOTransportWrapper {
  std::optional<IOTransport *> transport_ptr;
};

enum IOAddressType : uint8_t { IO_SOCKADDR, FILE_DESCRIPTOR, FILE_PATH, MAC };

struct IOAddress {
  IOAddressType addr_type;
  union {
    int fd;
    uint8_t sockaddr[16] = {0};
    name_type_t file_name_index;
  } val;
};

enum IOConnectionState : uint8_t {
  NOT_ACTIVE,
  DEAD,
  ALIVE,
};
enum IOAckState : uint8_t {
  ACK_NOT_ACTIVE,
  ACK_ACTIVE,
  ACK_ACKNOWLEGDED,
  MAX_ACK
};

std::expected<int, ErrorWrapper> process_io_res(CoRoutineCtxt *ctxt,
                                                IOMethod io_method,
                                                IOType io_type, int res);

struct SqePayload {
  union {
    struct {
      uint64_t listen_size;
      const void *addr;
      IOTCPType tcp_type;
    } socket_open;
    struct {
      int flag;
      uint32_t mode;
      const char *file_path;
    } file_open;
    struct {
      uint64_t buf_size;
      int fd;
      const void *addr;
      const uint8_t *buf;
    } write_payload;
    struct {
      uint64_t buf_size;
      int fd;
      void *addr;
      uint8_t *buf;
    } read_payload;
    struct {
      int fd;
      const void *conn_addr;
    } close_payload;
  };
};
struct IOSqe {
  SqePayload payload;
  const void *user_data = nullptr;
  IOMethod method;
  IOType io_type;
  IOPackageType io_package_type;
  bool with_ack;
};
struct IOCqe {
  int32_t res;
  const void *user_data = nullptr;
};
#define MAX_CQE_QUEUE_SIZE 256
#define MAX_GENERAL_SQE_SIZE 16

#define CONNECTION_WRITE_QUEUE_SIZE 56

struct SQEAck {
  IOSqe sqe = {};
  uint64_t timeout = 0;
  int32_t res;
  IOAckState state = ACK_NOT_ACTIVE;
  uint8_t ack_amount;
};
struct IOConnection {
  Queue<IOSqe, CONNECTION_WRITE_QUEUE_SIZE> write_queue = {};

  std::optional<IOSqe> read_sqe = std::nullopt;
  SQEAck ack = {};
  uint64_t last_out_sqe = 0;
  uint64_t last_in_sqe = 0;
  IOAddress io_addr = {};
  IOConnectionState connection_state = IOConnectionState::NOT_ACTIVE;
  IODeviceType device_type;
  uint8_t device_id = 0;
};

struct IOSQEConfig {
  uint8_t timeout_ms;
  uint8_t max_ack;
  virtual int write_function(IOSqe *sqe, IOConnection *connection,
                             uint8_t *out_buf) = 0;
  virtual int read_function(IOSqe *sqe) = 0;
  virtual int open_function(IOSqe *sqe) = 0;
  virtual int close_function(IOSqe *sqe) = 0;
  virtual int accept_function(IOSqe *sqe) = 0;
  virtual int connect_function(IOSqe *sqe) = 0;
};
class IOHandler {
protected:
  AllocatorInterface *transport_allocator;
  IOTransportWrapper transports[IOMethod::IO_END];

public:
  IOSQEConfig *sqe_configs[IOMethod::IO_END] = {};
  Queue<IOCqe, MAX_CQE_QUEUE_SIZE> cqe_queue;
  Queue<IOSqe, MAX_GENERAL_SQE_SIZE> sqe_queue;
  Queue<IOSqe, MAX_GENERAL_SQE_SIZE> pending_sqe_queue;
  IOHandler(AllocatorInterface *transport_allocator);
  IOTransport *register_transport(IOMethod io_method, uint64_t transport_size);
  template <typename T> T *get_transport(IOMethod io_method);
  virtual void process_all(uint64_t timeout) = 0;
};

template <typename T> T *IOHandler::get_transport(IOMethod io_method) {
  T *transport_ptr = (T *)this->transports[io_method].transport_ptr.value();
  return transport_ptr;
}

class IOConnectionHandler {
private:
public:
  IOConnection connections[IO_MAX_CONNECTIONS] = {};
  uint64_t max_connections = IO_MAX_CONNECTIONS;
  IOConnection *add_connection(const IOAddress *io_addr);
  IOConnection *get_or_add_connection(const IOAddress *io_addr);
  void close_connection(const IOAddress *io_addr);
  std::optional<IOConnection *> get_connection(const IOAddress *io_addr);
};
uint64_t parse_sqe_write(IOSqe *sqe, IOConnection *io_connection,
                         uint8_t *out_buf);
uint64_t parse_uart_write(IOSqe *sqe, IOConnection *io_connection,
                          uint8_t *out_buf);

#endif
