#ifndef STANDARD_METRICS_H
#define STANDARD_METRICS_H
#include "general/interfaces/utility/metrics.h"
#include <cstddef>
#include <cstdint>
#define METRIC_LOG_INTERVAL 200
struct StandardMetricHolder {
  size_t coroutines_created = 0;
  size_t coroutines_freed = 0;
  size_t coroutine_timeout = 0;
};
class StandardMetrics : public MetricsInterface {
private:
  StatMetric temp_stat_metrics[StatMetricType::LATENCY_METRIC_END] = {};
  StatMetric stat_metrics[StatMetricType::LATENCY_METRIC_END] = {};
  StandardMetricHolder metrics_holder = {};
  StandardMetricHolder temp_metrics_holder = {};
  void print_stat_metric(StatMetricType metric_type);
  void print_metric(MetricType metric_type);
  void reset_metrics();

public:
  StandardMetrics();
  void document_metric(MetricType metric_type) override;
  void document_statistics_metric_metric(StatMetricType metric_type,
                                         uint64_t latency_ns) override;
  uint64_t get_metric(MetricType metric_type) override;
  void print_metrics() override;
  void print_total_metrics() override;
};

#endif
