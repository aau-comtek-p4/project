#ifndef COMMON_H
#define COMMON_H

#define RANDOM_SEED 932

#define SERVER_TAG "SERVER"
#define SERVER_ERROR_TAG "SERVER ERROR"
#define NODE_TAG "NODE"
#define NODE_ERROR_TAG "NODE ERROR"

#include "general/misc/context.h"
#include <cstdint>

struct NodeContextSettings : public ContextSettings {
  static constexpr uint64_t max_coroutine_amount = 40;
  static constexpr uint64_t max_coroutine_size = 500;
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

struct ServerContextSettings : public ContextSettings {
  static constexpr uint64_t max_coroutine_amount = 40;
  static constexpr uint64_t max_coroutine_size = 500;
  static constexpr uint64_t max_buffer_amount = 20;
  static constexpr uint64_t max_buffer_size = 1024;
  static constexpr uint64_t max_deadlines = 20;
  static constexpr uint64_t max_queue_depth = 1024;
  static constexpr uint64_t max_loop_queue = 20;
  static constexpr uint64_t max_total_size = 1024 * 100;
  static constexpr uint64_t max_io_transport_size = 1024;
  static constexpr uint64_t random_seed = 932;
};

#endif
