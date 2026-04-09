#include "general/misc/context.h"

struct NodeContextSettings : public ContextSettings {
  static constexpr uint64_t clock_tick_ms = 10;
  static constexpr uint64_t max_total_size = 1024 * 60;
  static constexpr uint64_t max_coroutine_size = 400;
  static constexpr uint64_t max_coroutine_amount = 20;
  static constexpr uint64_t max_coroutine_generator_size = 30;
  static constexpr uint64_t max_coroutine_generator_amount = 20;
  static constexpr uint64_t max_ready_queue = 20;
  static constexpr uint64_t max_staging_queue = 20;
  static constexpr uint64_t max_buffer_size = 1024;
  static constexpr uint64_t max_buffer_amount = 20;
  static constexpr uint64_t max_deadlines = 20;
  static constexpr uint64_t max_queue_depth = 20;
  static constexpr uint64_t max_io_transport_size = 1024;
  static constexpr uint64_t max_trace_amount = 150;
};
