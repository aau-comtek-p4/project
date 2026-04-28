#include "esp32/logging/queue_logger.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "general/common.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/crc.h"
#include "general/misc/shutdown.h"
#include "hal/uart_types.h"
#include "portmacro.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <esp_rom_crc.h>
ESPSerialLogger::ESPSerialLogger(QueueInterface<LogEntry> *msg_queue,
                                 LogSerializerInterface *serializer)
    : msg_queue(msg_queue) {
  this->serializer = serializer;
}
void ESPSerialLogger::log_entry(LogEntry log_entry) noexcept {

  auto res = this->msg_queue->enque(std::move(log_entry));
  if (res.has_value()) {
    return;
  }
  if (log_entry.severity == LogLevel::LOG_ERROR) {
    auto _ = this->msg_queue->deque();
    res = this->msg_queue->enque(std::move(log_entry));
  }
  this->dropped_count += 1;
};

void ESPSerialLogger::submit(uint64_t timeout) noexcept {
  uint64_t start_time = program_ctxt->clock->rt_since_start_ns();
  uint64_t current_time = start_time;
  uint64_t last_time = current_time;
  uint64_t end_time = start_time + timeout;
  auto res = this->msg_queue->deque();
  int write_res = 0;
  UARTLogFrame log_frame = {};
  uint32_t check;
  uint64_t log_amount = 0;

  while (res.has_value() && end_time > current_time) {
    log_frame = {};
    log_frame.magic = UART_MAGIC_HEADER;
    log_frame.pad1 = 0;
    log_frame.entry = res.value();
    log_frame.pad2 = 0;
    check = esp_rom_crc32_le(CRC32_START, (uint8_t *)&log_frame,
                             sizeof(log_frame) - sizeof(uint32_t));
    log_frame.check = check;
    write_res = uart_write_bytes(UART_NUM_0, &log_frame, sizeof(log_frame));
    if (write_res < 0) {
      safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
    }
    res = this->msg_queue->deque();
    log_amount += 1;
    current_time = program_ctxt->clock->rt_since_start_ns();
    program_ctxt->metrics->document_statistics_metric_metric(
        StatMetricType::METRIC_SINGLE_LOG_TIME, current_time - last_time);
    last_time = current_time;
  }
  if (this->dropped_count > 0) {
    program_ctxt->metrics->document_statistics_metric_metric(
        StatMetricType::METRIC_LOG_DROP, dropped_count);
    this->dropped_count = 0;
  }
  program_ctxt->metrics->document_statistics_metric_metric(
      StatMetricType::METRIC_LOG_AMOUNT, log_amount);
  end_time = program_ctxt->clock->rt_since_start_ns();
  program_ctxt->metrics->document_statistics_metric_metric(
      StatMetricType::METRIC_LOG_TIME, end_time - start_time);
};
void ESPSerialLogger::submit() noexcept {
  uint64_t start_time = program_ctxt->clock->rt_since_start_ns();
  uint64_t current_time = start_time;
  uint64_t last_time = start_time;
  auto res = this->msg_queue->deque();
  int write_res = 0;
  UARTLogFrame log_frame = {};
  uint32_t check;
  uint64_t log_amount = 0;
  while (res.has_value()) {
    log_frame = {};
    log_frame.magic = UART_MAGIC_HEADER;
    log_frame.pad1 = 0;
    log_frame.entry = res.value();
    log_frame.pad2 = 0;
    check = esp_rom_crc32_le(CRC32_START, (uint8_t *)&log_frame,
                             sizeof(log_frame) - sizeof(uint32_t));
    log_frame.check = check;
    write_res = uart_write_bytes(UART_NUM_0, &log_frame, sizeof(log_frame));
    log_amount += 1;
    res = this->msg_queue->deque();
    current_time = program_ctxt->clock->rt_since_start_ns();
    program_ctxt->metrics->document_statistics_metric_metric(
        StatMetricType::METRIC_SINGLE_LOG_TIME, current_time - last_time);
    last_time = current_time;
  }
  if (write_res < 0) {
    safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
  }
  if (this->dropped_count > 0) {
    program_ctxt->metrics->document_statistics_metric_metric(
        StatMetricType::METRIC_LOG_DROP, dropped_count);
    this->dropped_count = 0;
  }
  program_ctxt->metrics->document_statistics_metric_metric(
      StatMetricType::METRIC_LOG_AMOUNT, log_amount);
  uint64_t end_time = program_ctxt->clock->rt_since_start_ns();
  program_ctxt->metrics->document_statistics_metric_metric(
      StatMetricType::METRIC_LOG_TIME, end_time - start_time);
};
