
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
    return this->metrics_holder.coroutines_freed -
           this->temp_metrics_holder.coroutines_freed;
  case MetricType::COROUTINE_CREATED:
    return this->metrics_holder.coroutines_created -
           this->temp_metrics_holder.coroutines_created;
  case MetricType::COROUTINE_TIMEOUT:
    return this->metrics_holder.coroutine_timeout -
           this->temp_metrics_holder.coroutine_timeout;
  }
  safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
  return 0;
}

void StandardMetrics::print_metric(MetricType metric_type) {
  program_ctxt->logger->log_entry(
      logging::log_metric(metric_type, this->get_metric(metric_type)));
}

void StandardMetrics::print_stat_metric(StatMetricType metric_type) {
  StatMetric temp_metric = this->stat_metrics[metric_type];
  temp_metric.total -= this->temp_stat_metrics[metric_type].total;
  temp_metric.count -= this->temp_stat_metrics[metric_type].count;
  temp_metric.worst_case = this->temp_stat_metrics[metric_type].worst_case;
  program_ctxt->logger->log_entry(
      logging::log_stat_metric(metric_type, temp_metric));
}
void StandardMetrics::print_total_metrics() {
  for (int i = 0; i < StatMetricType::LATENCY_METRIC_END; i++) {
    this->temp_stat_metrics[i].total = 0;
    this->temp_stat_metrics[i].count = 0;
    this->temp_stat_metrics[i].worst_case = this->stat_metrics[i].worst_case;
  }
  this->temp_metrics_holder.coroutine_timeout = 0;
  this->temp_metrics_holder.coroutines_created = 0;
  this->temp_metrics_holder.coroutines_freed = 0;
  this->print_metric(MetricType::COROUTINE_CREATED);
  this->print_metric(MetricType::COROUTINES_FREED);
  this->print_metric(MetricType::COROUTINE_TIMEOUT);

  for (int i = 0; i < StatMetricType::LATENCY_METRIC_END; i++) {
    this->print_stat_metric((StatMetricType)i);
  }
}
void StandardMetrics::print_metrics() {
  this->print_metric(MetricType::COROUTINE_CREATED);
  this->print_metric(MetricType::COROUTINES_FREED);
  this->print_metric(MetricType::COROUTINE_TIMEOUT);

  for (int i = 0; i < StatMetricType::LATENCY_METRIC_END; i++) {
    this->print_stat_metric((StatMetricType)i);
  }
  this->reset_metrics();
}
StandardMetrics::StandardMetrics() {}
void StandardMetrics::reset_metrics() {
  for (int i = 0; i < StatMetricType::LATENCY_METRIC_END; i++) {
    this->temp_stat_metrics[i].total = this->stat_metrics[i].total;
    this->temp_stat_metrics[i].count = this->stat_metrics[i].count;
    this->temp_stat_metrics[i].worst_case = 0;
  }
  this->temp_metrics_holder.coroutine_timeout =
      this->metrics_holder.coroutine_timeout;
  this->temp_metrics_holder.coroutines_created =
      this->metrics_holder.coroutines_created;
  this->temp_metrics_holder.coroutines_freed =
      this->metrics_holder.coroutines_freed;
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
  case StatMetricType::METRIC_COROUTINE_RESUME:
    return "coroutine_resume";
  case StatMetricType::METRIC_IO_PROCESSING:
    return "io_processing";
  case StatMetricType::METRIC_IO_SUBMIT:
    return "io_submit";
  case StatMetricType::METRIC_SEQ_MISSING:
    return "sqe_missing";
  case StatMetricType::LATENCY_METRIC_END:
    return "unkown";
  }
  return "unkown";
}
void StandardMetrics::document_statistics_metric_metric(
    StatMetricType metric_type, uint64_t latency) {
  this->stat_metrics[metric_type].count += 1;
  this->stat_metrics[metric_type].total += latency;
  if (this->temp_stat_metrics[metric_type].worst_case < latency) {
    this->temp_stat_metrics[metric_type].worst_case = latency;
  }
  if (this->stat_metrics[metric_type].worst_case < latency) {
    this->stat_metrics[metric_type].worst_case = latency;
  }
}
