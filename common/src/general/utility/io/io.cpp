#include "general/interfaces/io/io.h"
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cmath>
#include <cstdint>
#include <expected>

IOHandler::IOHandler(AllocatorInterface *transport_allocator) {
  this->transport_allocator = transport_allocator;
}

IOTransport *IOHandler::register_transport(IOMethod io_method,
                                           uint64_t transport_size) {
  auto res = this->transport_allocator->allocate(transport_size);
  if (!res.has_value()) {
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

void IOHandler::cancel(IOMethod io_method, const void *user_data) {
  this->transports[io_method].transport_ptr.value()->cancel(user_data);
};
std::expected<int, ErrorWrapper> process_io_res(CoRoutineCtxt *ctxt,
                                                IOMethod io_method,
                                                IOType io_type, int res) {
  uint64_t parent_index = ctxt->parent_ctxt ? ctxt->parent_ctxt->name_id : 0;
  if (ctxt->cancelled) {
    program_ctxt->logger->log_entry(logging::log_io_timeout(
        io_method, io_type, ctxt->name_id, parent_index, ctxt->trace.id));
    return std::unexpected(ErrorWrapper{.error = CustomErrors::TIMEOUT,
                                        .tag = ErrorWrapper::CUSTOM});
  }
  if (res < 0) {
    program_ctxt->logger->log_entry(logging::log_io_error(
        io_method, io_type, ctxt->name_id, parent_index, ctxt->trace.id, res));
    return std::unexpected(
        ErrorWrapper{.error = errno, .tag = ErrorWrapper::ERRNO});
  }
  if (IO_LOGGING) {
    program_ctxt->logger->log_entry(logging::log_io_complete(
        io_method, io_type, ctxt->name_id, parent_index, ctxt->trace.id, res));
  }
  return res;
}
