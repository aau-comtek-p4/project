#include "general/interfaces/io/io.h"
#include "general/common.h"
#include "general/interfaces/storage/allocator.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cmath>
#include <cstdint>
const char *get_io_type(IOType type) {
  switch (type) {
  case IOType::OPEN:
    return "OPEN";
  case IOType::READ:
    return "READ";
  case IOType::WRITE:
    return "WRITE";
  case IOType::CLOSE:
    return "CLOSE";
  case IOType::RECV:
    return "RECV";
  case IOType::ACCEPT:
    return "ACCEPT";
  case IOType::SEND:
    return "SEND";
  case IOType::CONNECT:
    return "CONNECT";
  }
  return "Unknown type";
}

IOHandler::IOHandler(AllocatorInterface *transport_allocator) {
  this->transport_allocator = transport_allocator;
}

IOTransport *IOHandler::register_transport(IOMethod io_method,
                                           uint64_t transport_size) {
  auto res = this->transport_allocator->allocate(transport_size);
  if (!res.has_value()) {
    program_ctxt->logger->log_err(
        IO_ERROR_TAG, "IO handler failed to register transport of type: [%s]",
        "hello");
    safe_shutdown(res.error());
  }

  auto io_transport_ptr = (IOTransport *)res.value();
  this->transports[io_method].transport_ptr = io_transport_ptr;
  return io_transport_ptr;
}

void IOHandler::submit_all() {
  for (uint64_t i = 0; i < IOMethod::IO_END; i++) {
    if (this->transports[i].transport_ptr.has_value()) {
      this->transports[i].transport_ptr.value()->submit();
    }
  }
}

void IOHandler::process_all(uint64_t timeout) {
  for (uint64_t i = 0; i < IOMethod::IO_END; i++) {
    if (this->transports[i].transport_ptr.has_value()) {
      this->transports[i].transport_ptr.value()->process(
          std::ceil(timeout / IOMethod::IO_END));
    }
  }
}

void IOHandler::cancel(const void *user_data) {
  for (uint64_t i = 0; i < IOMethod::IO_END; i++) {
    if (this->transports[i].transport_ptr.has_value()) {
      this->transports[i].transport_ptr.value()->cancel(user_data);
    }
  }
};
