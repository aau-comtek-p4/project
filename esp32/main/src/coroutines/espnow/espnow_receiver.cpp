#include "esp32/common.h"
#include "esp32/io/io.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/udp_transport.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/shutdown.h"
#include "lwip/sockets.h"

#include "esp32/coroutines/espnow.h"
#include <cstdint>
#include <cstdio>
#include <functional>
Job espnow_receiver_routine() {
  auto transport =
      program_ctxt->io->get_transport<UDPIOTransport>(IOMethod::IO_ESP_NOW);

  IOAddress cli_addr;
  cli_addr.addr_type = IOAddressType::MAC;

  auto buf = (uint8_t *)program_ctxt->buffer_allocator->allocate(1024).value();

  while (true) {
    auto res = co_await transport->recv_from(nullptr, &cli_addr, buf, 1024);
    if (!res.has_value()) {
      safe_shutdown(res.error());
    }

    auto package_header = (IOPackageHeader *)buf;
    auto data = (buf + sizeof(IOPackageHeader));

    IOConnection *connection =
        program_ctxt->connection_handler->get_connection(&cli_addr).value();

    if (package_header->seq < connection->last_in_sqe) {
      program_ctxt->logger->log_entry(logging::log_debug(
          "Old package id%" PRIu8, package_header->device_id));
      continue;
    }

    if (package_header->seq == connection->last_in_sqe &&
        connection->last_in_sqe != 0) {
      program_ctxt->logger->log_entry(
          logging::log_debug("Duplicate %" PRIu64 ", id %" PRIu8,
                             package_header->seq, package_header->device_id));
      continue;
    }
    if (package_header->seq > connection->last_in_sqe) {
      if (package_header->seq - connection->last_in_sqe > 1) {
        program_ctxt->metrics->document_statistics_metric_metric(
            StatMetricType::METRIC_SEQ_MISSING,
            package_header->seq - connection->last_in_sqe - 1);
        program_ctxt->logger->log_entry(logging::log_debug(
            "Package drop id %" PRIu8, package_header->device_id));
      }
      connection->last_in_sqe = package_header->seq;
    }

    if (package_header->with_ack) {
      auto res = co_await transport->send_to(nullptr, &cli_addr, nullptr, 0,
                                             IOPackageType::IOPackage_ACK);
      if (!res.has_value()) {
        safe_shutdown(res.error());
      }
    }
    program_ctxt->logger->log_entry(logging::log_debug(
        "Got package id:%" PRIu8, package_header->device_id));

    if (package_header->package_type == IOPackageType::IOPackage_ACK) {
      connection->ack.state = IOAckState::ACK_ACKNOWLEGDED;
    }
  }
}
