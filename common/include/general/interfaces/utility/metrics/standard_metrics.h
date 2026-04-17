#ifndef STANDARD_METRICS_H
#define STANDARD_METRICS_H
#include "general/interfaces/utility/metrics.h"
#include <cstddef>
#include <cstdint>
#define METRIC_LOG_INTERVAL 200
struct StandardMetricHolder {
  size_t packet_loss;
  size_t tick_miss;
  size_t surpassed_deadline;
  size_t failed_receive;
  size_t failed_connect;
  size_t failed_send;
  size_t message_sent;
  size_t message_received;
  size_t connection_received;
  size_t total_coroutines;
  size_t coroutines_freed;
  size_t accepted_connection;
  size_t failed_accept;
  size_t file_opened;
  size_t file_read;
  size_t file_written;
  size_t opened_failed;
  size_t write_failed;
  size_t read_failed;
  size_t closed_fd;
  size_t closed_failed;
  size_t disconnect;
  size_t real_tick;
};
class StandardMetrics : public MetricsInterface {
private:
  StatMetric stat_metrics[StatMetricType::LATENCY_METRIC_END];
  StandardMetricHolder metrics_holder;
  void print_metric(StatMetricType metric_type);

public:
  StandardMetrics();
  void document_metric(MetricType metric_type) override;
  void document_statistics_metric_metric(StatMetricType metric_type,
                                         uint64_t latency_ns) override;
  uint64_t get_metric(MetricType metric_type) override;
  void print_metrics() override;
};

#endif
