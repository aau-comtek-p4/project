#ifndef COMMON_H
#define COMMON_H

#include "general/misc/context.h"
#include "general/misc/names.h"
#include <cstdint>
#define LOG_FILE_NAME "log.txt"

struct NodeContextSettings : public ContextSettings {
  static constexpr uint64_t max_coroutine_amount = 40;
  static constexpr uint64_t max_coroutine_size = 600;
  static constexpr uint64_t max_buffer_amount = 20;
  static constexpr uint64_t max_buffer_size = 1024;
  static constexpr uint64_t max_deadlines = 20;
  static constexpr uint64_t max_queue_depth = 1024;
  static constexpr uint64_t max_loop_queue = 20;
  static constexpr uint64_t max_total_size = 1024 * 150;
  static constexpr uint64_t max_io_transport_size = 1024;
  static constexpr uint64_t max_log_amount = 200;
  static constexpr uint64_t random_seed = 932;
};

struct ServerContextSettings : public ContextSettings {
  static constexpr uint64_t max_coroutine_amount = 20;
  static constexpr uint64_t max_coroutine_size = 600;
  static constexpr uint64_t max_buffer_amount = 20;
  static constexpr uint64_t max_buffer_size = 1024;
  static constexpr uint64_t max_deadlines = 20;
  static constexpr uint64_t max_queue_depth = 1024;
  static constexpr uint64_t max_loop_queue = 20;
  static constexpr uint64_t max_total_size = 1024 * 100;
  static constexpr uint64_t max_io_transport_size = 1024;
  static constexpr uint64_t max_log_amount = 200;
  static constexpr uint64_t random_seed = 932;
};

#endif
