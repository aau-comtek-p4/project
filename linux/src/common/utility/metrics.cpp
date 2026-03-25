#include "general/interfaces/utility/metrics.h"
#include "common/utility/metric.h"
#include "general/common.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/shutdown.h"
#include <cstddef>
const char *get_metric_type(MetricType metric_type) {
  switch (metric_type) {
  case MetricType::CONNECTION_RECEIVED:
    return "CONNECTION RECEIVED";
  case MetricType::MESSAGE_RECEIVED:
    return "MESSAGE RECEIVED";
  case MetricType::MESSAGE_SENT:
    return "MESSAGE SENT";
  case MetricType::COROUTINES_FREED:
    return "COROUTINES FREED";
  case MetricType::PACKET_LOSS:
    return "PACKET LOSS";
  case MetricType::TICK_MISS:
    return "TICK MISS";
  case MetricType::SURPASSED_DEADLINE:
    return "SURPASSED DEADLINE";
  case MetricType::FAILED_RECEIVE:
    return "FAILED RECEIVE";
  case MetricType::FAILED_CONNECT:
    return "FAILED CONNECT";
  case MetricType::FAILED_SEND:
    return "FAILED SEND";
  case MetricType::TOTAL_COROUTINE:
    return "TOTAL COROUTINE";
  case MetricType::ACCEPTED_CONNECTION:
    return "ACCEPTED CONNECTION";
  case MetricType::FAILED_ACCEPT:
    return "FAILED ACCEPT";
  case MetricType::WRITE_FAILED:
    return "WRITE FAILED";
  case MetricType::FILE_WRITE:
    return "FILE WRITE";
  case MetricType::READ_FAILED:
    return "READ FAILED";
  case MetricType::FILE_READ:
    return "FILE READ";
  case MetricType::OPENED_FILE:
    return "OPENED FILE";
  case MetricType::FAILED_OPENED:
    return "FAILED OPENED";
  case MetricType::CLOSED_FD:
    return "CLOSED FD";
  case MetricType::CLOSED_FAILED:
    return "CLOSED FAILED";
  case MetricType::DISCONNECT:
    return "DISCONNECT";
  }
  return "UNKNOWN METRIC";
}

void LinuxMetric::document_metric(MetricType metric_type) {
  program_ctxt->logger->log_debug(METRIC_TAG, "Metric [%s] documented",
                                  get_metric_type(metric_type));
  switch (metric_type) {
  case MetricType::CONNECTION_RECEIVED:
    this->metrics_holder.connection_received += 1;
    return;
  case MetricType::MESSAGE_RECEIVED:
    this->metrics_holder.message_received += 1;
    return;
  case MetricType::MESSAGE_SENT:
    this->metrics_holder.message_sent += 1;
    return;
  case MetricType::COROUTINES_FREED:
    this->metrics_holder.coroutines_freed += 1;
    return;
  case MetricType::PACKET_LOSS:
    this->metrics_holder.packet_loss += 1;
    return;
  case MetricType::TICK_MISS:
    this->metrics_holder.tick_miss += 1;
    return;
  case MetricType::SURPASSED_DEADLINE:
    this->metrics_holder.surpassed_deadline += 1;
    return;
  case MetricType::FAILED_RECEIVE:
    this->metrics_holder.failed_receive += 1;
    return;
  case MetricType::FAILED_CONNECT:
    this->metrics_holder.failed_connect += 1;
    return;
  case MetricType::FAILED_SEND:
    this->metrics_holder.failed_send += 1;
    return;
  case MetricType::TOTAL_COROUTINE:
    this->metrics_holder.total_coroutines += 1;
    return;
  case MetricType::ACCEPTED_CONNECTION:
    this->metrics_holder.accepted_connection += 1;
    return;
  case MetricType::FAILED_ACCEPT:
    this->metrics_holder.failed_accept += 1;
    return;
  case MetricType::WRITE_FAILED:
    this->metrics_holder.write_failed += 1;
    return;
  case MetricType::READ_FAILED:
    this->metrics_holder.read_failed += 1;
    return;
  case MetricType::FAILED_OPENED:
    this->metrics_holder.opened_failed += 1;
    return;
  case MetricType::OPENED_FILE:
    this->metrics_holder.file_opened += 1;
    return;
  case MetricType::FILE_WRITE:
    this->metrics_holder.file_written += 1;
    return;
  case MetricType::FILE_READ:
    this->metrics_holder.file_read += 1;
    return;

  case MetricType::CLOSED_FD:
    this->metrics_holder.closed_fd += 1;
    return;
  case MetricType::CLOSED_FAILED:
    this->metrics_holder.closed_failed += 1;
    return;

  case MetricType::DISCONNECT:
    this->metrics_holder.disconnect += 1;
    return;
  }
}

size_t LinuxMetric::get_metric(MetricType metric_type) {
  switch (metric_type) {
  case MetricType::CONNECTION_RECEIVED:
    return this->metrics_holder.connection_received;
  case MetricType::MESSAGE_RECEIVED:
    return this->metrics_holder.message_received;
  case MetricType::MESSAGE_SENT:
    return this->metrics_holder.message_sent;
  case MetricType::COROUTINES_FREED:
    return this->metrics_holder.coroutines_freed;
  case MetricType::PACKET_LOSS:
    return this->metrics_holder.packet_loss;
  case MetricType::TICK_MISS:
    return this->metrics_holder.tick_miss;
  case MetricType::SURPASSED_DEADLINE:
    return this->metrics_holder.surpassed_deadline;
  case MetricType::FAILED_RECEIVE:
    return this->metrics_holder.failed_receive;
  case MetricType::FAILED_CONNECT:
    return this->metrics_holder.failed_connect;
  case MetricType::FAILED_SEND:
    return this->metrics_holder.failed_send;
  case MetricType::TOTAL_COROUTINE:
    return this->metrics_holder.total_coroutines;
  case MetricType::ACCEPTED_CONNECTION:
    return this->metrics_holder.accepted_connection;
  case MetricType::FAILED_ACCEPT:
    return this->metrics_holder.failed_accept;
  case MetricType::OPENED_FILE:
    return this->metrics_holder.file_opened;
  case MetricType::FILE_WRITE:
    return this->metrics_holder.file_written;
  case MetricType::FAILED_OPENED:
    return this->metrics_holder.opened_failed;
  case MetricType::WRITE_FAILED:
    return this->metrics_holder.write_failed;
  case MetricType::FILE_READ:
    return this->metrics_holder.file_read;
  case MetricType::READ_FAILED:
    return this->metrics_holder.read_failed;
  case MetricType::CLOSED_FD:
    return this->metrics_holder.closed_fd;
  case MetricType::CLOSED_FAILED:
    return this->metrics_holder.closed_failed;
  case MetricType::DISCONNECT:
    return this->metrics_holder.disconnect;
  }
  program_ctxt->logger->log_err(METRIC_ERROR_TAG, "Got unkown metric: [%u]",
                                metric_type);
  safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
  return 0;
}

void LinuxMetric::print_metric(MetricType metric_type) {
  program_ctxt->logger->log_info(METRIC_TAG, "[%s]: [%lu]",
                                 get_metric_type(metric_type),
                                 this->get_metric(metric_type));
}

void LinuxMetric::print_metrics() {
  this->print_metric(MetricType::CONNECTION_RECEIVED);
  this->print_metric(MetricType::MESSAGE_RECEIVED);
  this->print_metric(MetricType::MESSAGE_SENT);
  this->print_metric(MetricType::COROUTINES_FREED);
  this->print_metric(MetricType::PACKET_LOSS);
  this->print_metric(MetricType::TICK_MISS);
  this->print_metric(MetricType::SURPASSED_DEADLINE);
  this->print_metric(MetricType::FAILED_RECEIVE);
  this->print_metric(MetricType::FAILED_CONNECT);
  this->print_metric(MetricType::FAILED_SEND);
  this->print_metric(MetricType::TOTAL_COROUTINE);
  this->print_metric(MetricType::ACCEPTED_CONNECTION);
  this->print_metric(MetricType::FAILED_ACCEPT);
  this->print_metric(MetricType::OPENED_FILE);
  this->print_metric(MetricType::FILE_WRITE);
  this->print_metric(MetricType::FAILED_OPENED);
  this->print_metric(MetricType::WRITE_FAILED);
  this->print_metric(MetricType::FILE_READ);
  this->print_metric(MetricType::READ_FAILED);
  this->print_metric(MetricType::CLOSED_FD);
  this->print_metric(MetricType::CLOSED_FAILED);
  this->print_metric(MetricType::DISCONNECT);
}
