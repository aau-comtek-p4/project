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
  TRACE_CREATED,
  TRACE_FREED,
};

class MetricsInterface {

public:
  virtual void document_metric(MetricType metric_type) = 0;
  virtual uint64_t get_metric(MetricType metric_type) = 0;
  virtual void print_metrics() = 0;
};

#endif
