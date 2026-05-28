#ifndef METRICS_INTERFACE_h
#define METRICS_INTERFACE_h

#include <cstddef>
#include <cstdint>
#define METRIC_TAG "METRIC"
#define METRIC_ERROR_TAG "METRIC ERROR"
enum MetricType : uint8_t {
  COROUTINE_CREATED,
  COROUTINES_FREED,
  COROUTINE_TIMEOUT,
};
enum StatMetricType : uint8_t {
  METRIC_LOOP_TIME = 0,
  METRIC_LOG_TIME = 1,
  METRIC_SUSPEND_TIME = 2,
  METRIC_LOG_DROP = 3,
  METRIC_LOOP_OP_TIME = 4,
  METRIC_TIME_UNTIL_DEADLINE = 5,
  METRIC_SURPASSED_DEADLINE = 6,
  METRIC_LOOP_OP = 7,
  METRIC_LOG_AMOUNT = 8,
  METRIC_SINGLE_LOG_TIME = 9,
  METRIC_IO_PROCESSING = 10,
  METRIC_IO_SUBMIT = 11,
  METRIC_COROUTINE_RESUME = 12,
  METRIC_SEQ_MISSING = 13,
  LATENCY_METRIC_END = 14,
};
struct StatMetric {
  uint64_t total = 0;
  uint64_t count = 0;
  uint64_t worst_case = 0;
};

class MetricsInterface {

public:
  virtual void document_metric(MetricType metric_type) = 0;
  virtual void document_statistics_metric_metric(StatMetricType metric_type,
                                                 uint64_t latency_ns) = 0;
  virtual uint64_t get_metric(MetricType metric_type) = 0;
  virtual void print_metrics() = 0;
  virtual void print_total_metrics() = 0;
};

const char *parse_stat_metric_type(StatMetricType metric_type);
const char *parse_metric_type(MetricType metric_type);

#endif
