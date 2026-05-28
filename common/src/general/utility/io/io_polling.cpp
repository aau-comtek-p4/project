
#include "general/common.h"
#include "general/interfaces/io/io.h"

#include "general/interfaces/io/polling_io.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/shutdown.h"
#include <cerrno>
#include <cstdint>
#include <cstdio>

void poll_read(IOSqe *sqe, IOConnection *io_connection) {
  IOSQEConfig *writer_config = program_ctxt->io->sqe_configs[sqe->method];
  IOCqe cqe;
  cqe.user_data = sqe->user_data;
  int res = writer_config->read_function(sqe);
  if (-res == EAGAIN) {
    return;
  }
  io_connection->read_sqe.reset();
  cqe.res = res;
  // Enqueue cqe
  auto res2 = program_ctxt->io->cqe_queue.enque(std::move(cqe));
  if (!res2.has_value()) {
    safe_shutdown(res2.error());
  }
}
void poll_write(IOSqe *sqe, IOConnection *io_connection, uint8_t *out_buf) {
  IOSQEConfig *sqe_config = program_ctxt->io->sqe_configs[sqe->method];

  IOCqe cqe;
  cqe.user_data = sqe->user_data;
  int res = 0;
  switch (io_connection->ack.state) {
  case IOAckState::MAX_ACK: {
    io_connection->ack.state = IOAckState::ACK_NOT_ACTIVE;

    IOSqe sqe = io_connection->write_queue.deque().value();
    if (sqe.io_package_type == IOPackageType::IOPackage_LOG) {
      auto _ = program_ctxt->logger->msg_queue->deque().value();
    }
    io_connection->ack.ack_amount = 0;
    io_connection->last_out_sqe += 1;
    res = -ETIMEDOUT;
    break;
  }
  case IOAckState::ACK_ACKNOWLEGDED: {
    io_connection->ack.state = IOAckState::ACK_NOT_ACTIVE;
    IOSqe sqe = io_connection->write_queue.deque().value();
    if (sqe.io_package_type == IOPackageType::IOPackage_LOG) {
      auto _ = program_ctxt->logger->msg_queue->deque().value();
    }
    io_connection->last_out_sqe += 1;

    io_connection->ack.ack_amount = 0;
    res = io_connection->ack.res;
    break;
  }
  case IOAckState::ACK_ACTIVE: {
    if (program_ctxt->clock->rt_since_start_ns() < io_connection->ack.timeout) {
      return;
    }
    if (io_connection->ack.ack_amount >= sqe_config->max_ack) {
      io_connection->ack.state = IOAckState::MAX_ACK;
      return;
    }
    res = sqe_config->write_function(sqe, io_connection, out_buf);
    if (-res == EAGAIN) {
      return;
    }
    if (res < 0) {
      break;
    }
    io_connection->ack.ack_amount += 1;
    io_connection->ack.timeout = program_ctxt->clock->rt_since_start_ns() +
                                 sqe_config->timeout_ms * NS_PR_MS;
    io_connection->ack.res = res;
    return;
  }
  case IOAckState::ACK_NOT_ACTIVE: {
    res = sqe_config->write_function(sqe, io_connection, out_buf);
    if (-res == EAGAIN) {
      return;
    }

    if (res < 0) {
      break;
    }
    if (sqe->with_ack) {
      memcpy(&io_connection->ack.sqe, sqe, sizeof(IOSqe));
      io_connection->ack.timeout = program_ctxt->clock->rt_since_start_ns() +
                                   sqe_config->timeout_ms * NS_PR_MS;
      io_connection->ack.res = res;
      io_connection->ack.state = IOAckState::ACK_ACTIVE;

      io_connection->ack.ack_amount = 0;
      return;
    } else {
      auto _ = io_connection->write_queue.deque();
    }
    io_connection->last_out_sqe += 1;

    break;
  }
  }
  cqe.res = res;
  // Enque cqe
  auto res2 = program_ctxt->io->cqe_queue.enque(std::move(cqe));
  if (!res2.has_value()) {
    safe_shutdown(res2.error());
  }
}

void poll_connection(IOConnection *io_connection, uint8_t *out_buf) {
  if (io_connection->read_sqe.has_value()) {
    poll_read(&io_connection->read_sqe.value(), io_connection);
  }

  auto possible_write_sqe = io_connection->write_queue.peek();
  if (possible_write_sqe.has_value()) {

    poll_write(&possible_write_sqe.value(), io_connection, out_buf);
  }
}
void poll_connections(uint8_t *out_buf) {
  for (uint64_t i = 0; i < program_ctxt->connection_handler->max_connections;
       i++) {
    if (program_ctxt->connection_handler->connections[i].connection_state !=
        IOConnectionState::ALIVE) {
      continue;
    }
    poll_connection(&program_ctxt->connection_handler->connections[i], out_buf);
  }
}

void general_poll_handle(IOSqe sqe, uint8_t *out_buf) {
  IOSQEConfig *sqe_config = program_ctxt->io->sqe_configs[sqe.method];
  IOCqe cqe;
  cqe.user_data = sqe.user_data;
  int res = 0;
  switch (sqe.io_type) {
  case IOType::ACCEPT: {
    res = sqe_config->accept_function(&sqe);
    if (-res == EAGAIN) {
      auto res = program_ctxt->io->pending_sqe_queue.enque(std::move(sqe));
      if (!res.has_value()) {
        safe_shutdown(res.error());
      }
      return;
    }
    break;
  }
  case IOType::CLOSE: {
    res = sqe_config->close_function(&sqe);
    break;
  }
  case IOType::OPEN: {
    res = sqe_config->open_function(&sqe);
    break;
  }
  case IOType::RECV: {
    res = sqe_config->read_function(&sqe);
    if (-res == EAGAIN) {
      auto res = program_ctxt->io->pending_sqe_queue.enque(std::move(sqe));
      if (!res.has_value()) {
        safe_shutdown(res.error());
      }
      return;
    }
    break;
  }
  case IOType::CONNECT: {
    res = sqe_config->connect_function(&sqe);
    break;
  }
  case IOType::READ: {
    res = sqe_config->read_function(&sqe);
    if (-res == EAGAIN) {
      auto res = program_ctxt->io->pending_sqe_queue.enque(std::move(sqe));
      if (!res.has_value()) {
        safe_shutdown(res.error());
      }
      return;
    }
    break;
  }
  default: {
    return;
  }
  }
  cqe.res = res;

  auto final_res = program_ctxt->io->cqe_queue.enque(std::move(cqe));
  if (!final_res.has_value()) {
    safe_shutdown(final_res.error());
  }
}

void poll_general(uint8_t *out_buf) {
  while (true) {
    auto res = program_ctxt->io->sqe_queue.deque();
    if (!res.has_value()) {
      break;
    }
    auto res2 =
        program_ctxt->io->pending_sqe_queue.enque(std::move(res.value()));
    if (!res2.has_value()) {
      safe_shutdown(res2.error());
    }
  }
  uint64_t pending_amount =
      program_ctxt->io->pending_sqe_queue.get_item_amount();
  for (uint64_t i = 0; i < pending_amount; i++) {
    auto res = program_ctxt->io->pending_sqe_queue.deque();
    if (!res.has_value()) {
      safe_shutdown(res.error());
    }
    IOSqe sqe = res.value();
    general_poll_handle(sqe, out_buf);
  }
}
BusyPollingIO::BusyPollingIO(AllocatorInterface *transport_allocator)
    : IOHandler(transport_allocator) {}

IOTransport *BusyPollingIO::register_transport(IOMethod io_method,
                                               uint64_t transport_size) {
  return IOHandler::register_transport(io_method, transport_size);
}

void BusyPollingIO::process_all(uint64_t timeout) {
  IOCqe cqe;
  uint64_t cqe_ready = program_ctxt->io->cqe_queue.get_item_amount();
  for (uint64_t i = 0; i < cqe_ready; i++) {

    auto res = program_ctxt->io->cqe_queue.deque();

    if (!res.has_value()) {
      safe_shutdown(res.error());
    }
    cqe = res.value();
    if (cqe.user_data != nullptr) {
      auto interface = (BasicIOAwaiterInterface *)cqe.user_data;
      interface->result = cqe.res;
      interface->handle.resume();
    }
  }
}
