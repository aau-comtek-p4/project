#ifndef METRICS_INTERFACE_h
#define METRICS_INTERFACE_h

#include <cstdint>
enum MetricType {
  PACKET_LOSS,
  TICK_MISS,
  SURPASSED_DEADLINE,

};

class MetricsInterface {
  virtual void document_metric(MetricType metric_type) = 0;
  virtual uint64_t get_metric(MetricType metric_type) = 0;
};

#endif
