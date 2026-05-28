#ifndef ESP32_COMMON_H
#define ESP32_COMMON_H

#include "freertos/idf_additions.h"
#include "general/misc/context.h"
#include "hal/uart_types.h"
#include "io/io.h"
#include <atomic>
#include <cstdint>

#define UART_RX_BUF_SIZE (4 * 1024)
#define UART_TX_BUF_SIZE (8 * 1024)
#define UART_QUEUE_SIZE 20

#define SQE_QUEUE_SIZE 1024

#define TASK1_STACK_SIZE 4096
#define TASK2_STACK_SIZE 16384

#define TASK1_PRIORITY 5
#define TASK2_PRIORITY 5

#define TASK1_CORE 0
#define TASK2_CORE 1

struct ESPContextSettings : public ContextSettings {
  static constexpr uint64_t max_coroutine_amount = 40;
  static constexpr uint64_t max_coroutine_size = 400;
  static constexpr uint64_t max_buffer_amount = 20;
  static constexpr uint64_t max_buffer_size = 1024;
  static constexpr uint64_t max_deadlines = 20;
  static constexpr uint64_t max_queue_depth = 1024;
  static constexpr uint64_t max_loop_queue = 20;
  static constexpr uint64_t max_total_size = 1024 * 100;
  static constexpr uint64_t max_io_transport_size = 1024;
  static constexpr uint64_t max_log_amount = 250;
  static constexpr uint64_t random_seed = 932;
};
inline bool esp_started = false;

inline uint32_t sensor_data = 42;
inline uint8_t total_id = 0;

#endif
