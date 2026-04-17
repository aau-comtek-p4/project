#ifndef METRICS_INTERFACE_h
#define METRICS_INTERFACE_h

#include <cstddef>
#include <cstdint>
#define METRIC_TAG "METRIC"
#define METRIC_ERROR_TAG "METRIC ERROR"
enum MetricType {
  PACKET_LOSS,
  TICK_MISS,
  SURPASSED_DEADLINE,
  FAILED_RECEIVE,
  FAILED_CONNECT,
  FAILED_SEND,
  MESSAGE_SENT,
  MESSAGE_RECEIVED,
  CONNECTION_RECEIVED,
  COROUTINE_CREATED,
  COROUTINES_FREED,
  ACCEPTED_CONNECTION,
  FAILED_ACCEPT,
  OPENED_FILE,
  FAILED_OPENED,
  FILE_WRITE,
  WRITE_FAILED,
  FILE_READ,
  READ_FAILED,
  CLOSED_FD,
  CLOSED_FAILED,
  DISCONNECT,
  REAL_TICK,
};
enum StatMetricType {
  METRIC_LOOP_TIME = 0,
  METRIC_LOG_TIME = 1,
  METRIC_SUSPEND_TIME = 2,
  METRIC_LOG_DROP = 3,
  LATENCY_METRIC_END = 4,
};
struct StatMetric {
  uint64_t total = 0;
  uint64_t count = 0;
  uint64_t worst_case = 0;
  uint64_t print_interval = 0;
};

class MetricsInterface {

public:
  virtual void document_metric(MetricType metric_type) = 0;
  virtual void document_statistics_metric_metric(StatMetricType metric_type,
                                                 uint64_t latency_ns) = 0;
  virtual uint64_t get_metric(MetricType metric_type) = 0;
  virtual void print_metrics() = 0;
};

const char *parse_latency_metric_type(StatMetricType metric_type);

#endif
