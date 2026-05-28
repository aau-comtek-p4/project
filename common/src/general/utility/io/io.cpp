#include "general/interfaces/io/io.h"
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/crc.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <expected>

const char *BasicIOAwaiterInterface::get_type() {
  return parse_io_type(this->type);
};
bool BasicIOAwaiterInterface::await_ready() { return false; }
bool BasicIOAwaiterInterface::await_suspend(
    std::coroutine_handle<Task<std::expected<int, ErrorWrapper>>::promise_type>
        handle) {
  if (handle.promise().ctxt.cancelled) {
    this->result = -ECANCELED;
    return false;
  }
  handle.promise().ctxt.io_address = this;
  this->handle = handle;
  this->submit();
  return true;
}
int BasicIOAwaiterInterface::await_resume() { return this->result; }

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
        ErrorWrapper{.error = -res, .tag = ErrorWrapper::ERRNO});
  }
  if (IO_LOGGING) {
    program_ctxt->logger->log_entry(logging::log_io_complete(
        io_method, io_type, ctxt->name_id, parent_index, ctxt->trace.id, res));
  }

  return res;
}
IOConnection *IOConnectionHandler::add_connection(const IOAddress *io_addr) {
  for (uint64_t i = 0; i < this->max_connections; i++) {
    if (this->connections[i].connection_state ==
        IOConnectionState::NOT_ACTIVE) {
      while (this->connections[i].write_queue.deque().has_value()) {
      }
      memcpy(&this->connections[i].io_addr, io_addr, sizeof(IOAddress));
      this->connections[i].connection_state = IOConnectionState::ALIVE;
      this->connections[i].last_in_sqe = 0;
      this->connections[i].last_out_sqe = 0;

      return &this->connections[i];
    }
  }
  safe_shutdown(ErrorWrapper{.error = -1, .tag = ErrorWrapper::CUSTOM});
  return nullptr;
}
void IOConnectionHandler::close_connection(const IOAddress *io_addr) {
  for (uint64_t i = 0; i < this->max_connections; i++) {
    if (!memcmp(&this->connections[i].io_addr, io_addr, sizeof(IOAddress))) {
      while (this->connections[i].write_queue.deque().has_value()) {
      }
      this->connections[i].read_sqe.reset();
      this->connections[i].connection_state = IOConnectionState::NOT_ACTIVE;
      return;
    }
  }
}
std::optional<IOConnection *>
IOConnectionHandler::get_connection(const IOAddress *io_addr) {

  for (uint64_t i = 0; i < this->max_connections; i++) {
    if (this->connections[i].connection_state ==
        IOConnectionState::NOT_ACTIVE) {
      continue;
    }

    if (!memcmp(&this->connections[i].io_addr, io_addr, sizeof(IOAddress))) {

      return &this->connections[i];
    }
  }
  return {};
}

IOConnection *
IOConnectionHandler::get_or_add_connection(const IOAddress *io_addr) {
  for (uint64_t i = 0; i < this->max_connections; i++) {
    if (!memcmp(&this->connections[i].io_addr, io_addr, sizeof(IOAddress))) {
      return &this->connections[i];
    }
  }
  return this->add_connection(io_addr);
}
uint64_t parse_sqe_write(IOSqe *sqe, IOConnection *io_connection,
                         uint8_t *out_buf) {
  IOPackageHeader package_header;
  package_header.device_type = (IODeviceType)device_type;
  package_header.package_type = sqe->io_package_type;
  package_header.device_id = device_id;
  package_header.seq = io_connection->last_out_sqe;
  package_header.with_ack = sqe->with_ack;

  memcpy(out_buf, &package_header, sizeof(package_header));
  if (sqe->io_package_type != IOPackageType::IOPackage_LOG) {

    memcpy(out_buf + sizeof(package_header), sqe->payload.write_payload.buf,
           sqe->payload.write_payload.buf_size);

    return sizeof(package_header) + sqe->payload.write_payload.buf_size;
  }
  LogEntry log_entry;
  if (sqe->with_ack) {
    log_entry = program_ctxt->logger->msg_queue->peek().value();
  } else {
    log_entry = program_ctxt->logger->msg_queue->deque().value();
  }

  memcpy(out_buf + sizeof(package_header), &log_entry, sizeof(log_entry));
  return sizeof(package_header) + sizeof(log_entry);
}

uint64_t parse_uart_write(IOSqe *sqe, IOConnection *io_connection,
                          uint8_t *out_buf) {
  IOPackageHeader package_header;
  package_header.device_type = (IODeviceType)device_type;
  package_header.package_type = sqe->io_package_type;
  package_header.device_id = device_id;
  package_header.seq = io_connection->last_out_sqe;
  package_header.with_ack = sqe->with_ack;

  IOUARTFrameHeader uart_package_header;
  uart_package_header.package_header = package_header;
  uart_package_header.magic = UART_MAGIC_HEADER;

  memcpy(out_buf, &uart_package_header, sizeof(uart_package_header));

  if (sqe->io_package_type != IOPackageType::IOPackage_LOG) {
    memcpy(out_buf + sizeof(uart_package_header),
           sqe->payload.write_payload.buf, sqe->payload.write_payload.buf_size);
    uint32_t crc = CRC32(out_buf, sizeof(uart_package_header) +
                                      sqe->payload.write_payload.buf_size);
    memcpy(out_buf + sizeof(uart_package_header) +
               sqe->payload.write_payload.buf_size,
           &crc, sizeof(crc));

    return sizeof(uart_package_header) + sqe->payload.write_payload.buf_size +
           sizeof(crc);
  }

  LogEntry log_entry;
  if (sqe->with_ack) {
    log_entry = program_ctxt->logger->msg_queue->peek().value();
  } else {
    log_entry = program_ctxt->logger->msg_queue->deque().value();
  }

  memcpy(out_buf + sizeof(uart_package_header), &log_entry, sizeof(log_entry));
  uint32_t crc =
      CRC32(out_buf, sizeof(uart_package_header) + sizeof(log_entry));
  memcpy(out_buf + sizeof(uart_package_header) + sizeof(log_entry), &crc,
         sizeof(crc));
  return sizeof(uart_package_header) + sizeof(log_entry) + sizeof(crc);
}
