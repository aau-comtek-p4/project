#ifndef ESP32_COMMON_H
#define ESP32_COMMON_H

#include "freertos/idf_additions.h"
#include "general/misc/context.h"
#include <atomic>
#include <cstdint>

struct ESPContextSettings : public ContextSettings {
  static constexpr uint64_t max_coroutine_amount = 40;
  static constexpr uint64_t max_coroutine_size = 500;
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

#endif
