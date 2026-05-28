#ifndef COMMON_POLLING_IO_H
#define COMMON_POLLING_IO_H
#include "general/interfaces/io/io.h"

struct TCPSQEConfig : public IOSQEConfig {
  int write_function(IOSqe *sqe, IOConnection *connection,
                     uint8_t *out_buf) override;
  int read_function(IOSqe *sqe) override;
  int open_function(IOSqe *sqe) override;
  int close_function(IOSqe *sqe) override;
  int accept_function(IOSqe *sqe) override;
  int connect_function(IOSqe *sqe) override;
};
struct UDPSQEConfig : public IOSQEConfig {
  int write_function(IOSqe *sqe, IOConnection *connection,
                     uint8_t *out_buf) override;
  int read_function(IOSqe *sqe) override;
  int open_function(IOSqe *sqe) override;
  int close_function(IOSqe *sqe) override;
  int accept_function(IOSqe *sqe) override;
  int connect_function(IOSqe *sqe) override;
};

struct SerialSQEConfig : public IOSQEConfig {
  int write_function(IOSqe *sqe, IOConnection *connection,
                     uint8_t *out_buf) override;
  int read_function(IOSqe *sqe) override;
  int open_function(IOSqe *sqe) override;
  int close_function(IOSqe *sqe) override;
  int accept_function(IOSqe *sqe) override;
  int connect_function(IOSqe *sqe) override;
};

struct FileSQEConfig : public IOSQEConfig {
  int write_function(IOSqe *sqe, IOConnection *connection,
                     uint8_t *out_buf) override;
  int read_function(IOSqe *sqe) override;
  int open_function(IOSqe *sqe) override;
  int close_function(IOSqe *sqe) override;
  int accept_function(IOSqe *sqe) override;
  int connect_function(IOSqe *sqe) override;
};

void poll_general(uint8_t *out_buf);
void poll_connections(uint8_t *out_buf);
class BusyPollingIO : public IOHandler {

public:
  BusyPollingIO(AllocatorInterface *transport_allocator);
  IOTransport *register_transport(IOMethod io_method, uint64_t transport_size);
  template <typename T> T *get_transport(IOMethod io_method);
  void process_all(uint64_t timeout) override;
};

template <typename T> T *BusyPollingIO::get_transport(IOMethod io_method) {
  T *transport_ptr = (T *)this->transports[io_method].transport_ptr.value();
  return transport_ptr;
}

#endif
