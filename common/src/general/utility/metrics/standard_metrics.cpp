
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
const char *parse_metric_type(MetricType metric_type) {
  switch (metric_type) {
  case MetricType::COROUTINES_FREED:
    return "coroutiness_freed";
  case MetricType::COROUTINE_CREATED:
    return "coroutines_created";
  case MetricType::COROUTINE_TIMEOUT:
    return "coroutine_timeouts";
  }
  return "UNKNOWN METRIC";
}

void StandardMetrics::document_metric(MetricType metric_type) {
  switch (metric_type) {
  case MetricType::COROUTINES_FREED:
    this->metrics_holder.coroutines_freed += 1;
    return;
  case MetricType::COROUTINE_CREATED:
    this->metrics_holder.coroutines_created += 1;
    return;
  case MetricType::COROUTINE_TIMEOUT:
    this->metrics_holder.coroutine_timeout += 1;
    return;
  }
  safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
}

uint64_t StandardMetrics::get_metric(MetricType metric_type) {
  switch (metric_type) {
  case MetricType::COROUTINES_FREED:
    return this->metrics_holder.coroutines_freed;
  case MetricType::COROUTINE_CREATED:
    return this->metrics_holder.coroutines_created;
  case MetricType::COROUTINE_TIMEOUT:
    return this->metrics_holder.coroutine_timeout;
  }
  safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
  return 0;
}

void StandardMetrics::print_metric(MetricType metric_type) {
  program_ctxt->logger->log_entry(
      logging::log_metric(metric_type, this->get_metric(metric_type)));
}

void StandardMetrics::print_stat_metric(StatMetricType metric_type) {
  program_ctxt->logger->log_entry(
      logging::log_stat_metric(metric_type, this->stat_metrics[metric_type]));
}
void StandardMetrics::print_metrics() {
  this->print_metric(MetricType::COROUTINE_CREATED);
  this->print_metric(MetricType::COROUTINES_FREED);
  this->print_metric(MetricType::COROUTINE_TIMEOUT);
  this->print_stat_metric(StatMetricType::METRIC_LOG_DROP);
  this->print_stat_metric(StatMetricType::METRIC_SUSPEND_TIME);
  this->print_stat_metric(StatMetricType::METRIC_LOOP_TIME);
  this->print_stat_metric(StatMetricType::METRIC_LOG_TIME);
  this->print_stat_metric(StatMetricType::METRIC_LOOP_OP_TIME);
  this->print_stat_metric(StatMetricType::METRIC_TIME_UNTIL_DEADLINE);
  this->print_stat_metric(StatMetricType::METRIC_SURPASSED_DEADLINE);
  this->print_stat_metric(StatMetricType::METRIC_LOOP_OP);
  this->print_stat_metric(StatMetricType::METRIC_LOG_AMOUNT);
  this->print_stat_metric(StatMetricType::METRIC_SINGLE_LOG_TIME);
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
  this->stat_metrics[StatMetricType::METRIC_LOOP_OP_TIME].print_interval = 0;
  this->stat_metrics[StatMetricType::METRIC_TIME_UNTIL_DEADLINE]
      .print_interval = 0;
  this->stat_metrics[StatMetricType::METRIC_SURPASSED_DEADLINE].print_interval =
      0;
  this->stat_metrics[StatMetricType::METRIC_LOOP_OP].print_interval = 0;
  this->stat_metrics[StatMetricType::METRIC_SINGLE_LOG_TIME].print_interval = 0;
  this->stat_metrics[StatMetricType::METRIC_LOG_AMOUNT].print_interval = 0;
}
const char *parse_stat_metric_type(StatMetricType metric_type) {
  switch (metric_type) {
  case StatMetricType::METRIC_LOG_TIME:
    return "log_time";
  case StatMetricType::METRIC_LOOP_TIME:
    return "loop_time";
  case StatMetricType::METRIC_SUSPEND_TIME:
    return "suspend_time";
  case StatMetricType::METRIC_LOG_DROP:
    return "dropped_log";
  case StatMetricType::METRIC_LOOP_OP_TIME:
    return "loop_op_time";
  case StatMetricType::METRIC_TIME_UNTIL_DEADLINE:
    return "deadline_diff";
  case StatMetricType::METRIC_SURPASSED_DEADLINE:
    return "surpasssed_deadline";
  case StatMetricType::METRIC_LOOP_OP:
    return "loop_op_amount";
  case StatMetricType::METRIC_LOG_AMOUNT:
    return "log_amount";
  case StatMetricType::METRIC_SINGLE_LOG_TIME:
    return "log_time_single";
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

  if (this->stat_metrics[metric_type].count %
          this->stat_metrics[metric_type].print_interval ==
      0) {
    program_ctxt->logger->log_entry(
        logging::log_stat_metric(metric_type, this->stat_metrics[metric_type]));
  }
}
