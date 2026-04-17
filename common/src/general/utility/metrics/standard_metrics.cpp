
#include "general/interfaces/utility/metrics/standard_metrics.h"
#include "general/common.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/context.h"
#include "general/misc/shutdown.h"
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
const char *get_metric_type(MetricType metric_type) {
  switch (metric_type) {
  case MetricType::CONNECTION_RECEIVED:
    return "connection_received";
  case MetricType::MESSAGE_RECEIVED:
    return "message_received";
  case MetricType::MESSAGE_SENT:
    return "message_sent";
  case MetricType::COROUTINES_FREED:
    return "coroutines_freed";
  case MetricType::PACKET_LOSS:
    return "packet_loss";
  case MetricType::TICK_MISS:
    return "tick_miss";
  case MetricType::SURPASSED_DEADLINE:
    return "surpassed_deadline";
  case MetricType::FAILED_RECEIVE:
    return "failed_receive";
  case MetricType::FAILED_CONNECT:
    return "failed_connect";
  case MetricType::FAILED_SEND:
    return "failed_send";
  case MetricType::COROUTINE_CREATED:
    return "coroutine_created";
  case MetricType::ACCEPTED_CONNECTION:
    return "accepted_connection";
  case MetricType::FAILED_ACCEPT:
    return "failed_accept";
  case MetricType::WRITE_FAILED:
    return "write_failed";
  case MetricType::FILE_WRITE:
    return "file_write";
  case MetricType::READ_FAILED:
    return "read_failed";
  case MetricType::FILE_READ:
    return "file-read";
  case MetricType::OPENED_FILE:
    return "opened_file";
  case MetricType::FAILED_OPENED:
    return "failed_open";
  case MetricType::CLOSED_FD:
    return "closed_fd";
  case MetricType::CLOSED_FAILED:
    return "close_failed";
  case MetricType::DISCONNECT:
    return "disconnect";
  case MetricType::REAL_TICK:
    return "real_tick";
  }
  return "UNKNOWN METRIC";
}

void StandardMetrics::document_metric(MetricType metric_type) {
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
  case MetricType::COROUTINE_CREATED:
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

  case MetricType::REAL_TICK:
    this->metrics_holder.real_tick += 1;
    return;
  }
}

uint64_t StandardMetrics::get_metric(MetricType metric_type) {
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
  case MetricType::COROUTINE_CREATED:
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
  case MetricType::REAL_TICK:
    return this->metrics_holder.real_tick;
  }
  safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
  return 0;
}

void StandardMetrics::print_metric(StatMetricType metric_type) {
  uint64_t avg = this->stat_metrics[metric_type].count
                     ? this->stat_metrics[metric_type].total /
                           this->stat_metrics[metric_type].count
                     : 0;
  program_ctxt->logger->log_entry(
      logging::log_stat_metric(metric_type, avg

                               ,
                               this->stat_metrics[metric_type].worst_case));
}
void StandardMetrics::print_metrics() {
  this->print_metric(StatMetricType::METRIC_LOG_DROP);
  this->print_metric(StatMetricType::METRIC_SUSPEND_TIME);
  this->print_metric(StatMetricType::METRIC_LOOP_TIME);
  this->print_metric(StatMetricType::METRIC_LOG_TIME);
}
StandardMetrics::StandardMetrics() {
  /*
this->stat_metrics[StatMetricType::METRIC_LOOP_TIME].print_interval = 4000;
this->stat_metrics[StatMetricType::METRIC_LOG_TIME].print_interval = 100;
this->stat_metrics[StatMetricType::METRIC_SUSPEND_TIME].print_interval = 10;
this->stat_metrics[StatMetricType::METRIC_LOG_DROP].print_interval = 1;
*/
  this->stat_metrics[StatMetricType::METRIC_LOOP_TIME].print_interval = 0;
  this->stat_metrics[StatMetricType::METRIC_LOG_TIME].print_interval = 0;
  this->stat_metrics[StatMetricType::METRIC_SUSPEND_TIME].print_interval = 0;
  this->stat_metrics[StatMetricType::METRIC_LOG_DROP].print_interval = 0;
}
const char *parse_latency_metric_type(StatMetricType metric_type) {
  switch (metric_type) {
  case StatMetricType::METRIC_LOG_TIME:
    return "log_time";
  case StatMetricType::METRIC_LOOP_TIME:
    return "loop_time";
  case StatMetricType::METRIC_SUSPEND_TIME:
    return "suspend_time";
  case StatMetricType::METRIC_LOG_DROP:
    return "dropped_log";
  case StatMetricType::LATENCY_METRIC_END:
    return "unkown";
  }
  return "unkown";
}
void StandardMetrics::document_statistics_metric_metric(
    StatMetricType metric_type, uint64_t latency) {
  this->stat_metrics[metric_type].count += 1;
  this->stat_metrics[metric_type].total += latency;
  if (this->stat_metrics[metric_type].worst_case < latency) {
    this->stat_metrics[metric_type].worst_case = latency;
  }
  if (this->stat_metrics[metric_type].print_interval == 0) {
    return;
  }

  uint64_t avg = this->stat_metrics[metric_type].count
                     ? this->stat_metrics[metric_type].total /
                           this->stat_metrics[metric_type].count
                     : 0;
  if (this->stat_metrics[metric_type].count %
          this->stat_metrics[metric_type].print_interval ==
      0) {
    program_ctxt->logger->log_entry(logging::log_stat_metric(
        metric_type, avg, this->stat_metrics[metric_type].worst_case));
  }
}
