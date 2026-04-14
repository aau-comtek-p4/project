#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include "netinet/in.h"
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <sys/types.h>

#define IO_TAG "IO"
#define IO_ERROR_TAG "IO ERROR"

class IOAwaitInterface {
public:
  IOType type;
  IOMethod io_method;
  std::coroutine_handle<> handle;
  virtual void set_result(int result) = 0;
  virtual const char *get_type() = 0;
  virtual bool await_ready() = 0;
  virtual void
  await_suspend(std::coroutine_handle<
                Task<std::expected<int, ErrorWrapper>>::promise_type>
                    handle) = 0;
  virtual int await_resume() = 0;
};

class IOTransport {
public:
  virtual void cancel(const void *user_data) = 0;
  virtual void submit() = 0;
  virtual void process(uint64_t timeout) = 0;
};

struct IOTransportWrapper {
  std::optional<IOTransport *> transport_ptr;
};
template <typename T> class Task;

struct IOAddress;
class IOHandler {
private:
  AllocatorInterface *transport_allocator;
  IOTransportWrapper transports[IOMethod::IO_END];

public:
  IOHandler(AllocatorInterface *transport_allocator);
  IOTransport *register_transport(IOMethod io_method, uint64_t transport_size);
  template <typename T> T *get_transport(IOMethod io_method);
  void submit_all();
  void process_all(uint64_t timeout);
  void cancel(IOMethod io_method, const void *user_data);
};

template <typename T> T *IOHandler::get_transport(IOMethod io_method) {
  T *transport_ptr = (T *)this->transports[io_method].transport_ptr.value();
  return transport_ptr;
}

#endif
