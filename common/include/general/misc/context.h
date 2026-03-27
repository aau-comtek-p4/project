#ifndef CONTEXT_H
#define CONTEXT_H
#include "general/common.h"
#include "general/interfaces/simulator/random.h"
#include "general/interfaces/simulator/random/seeded_random.h"
#include <cstdint>
enum CtxtIOType {
  LINUX,
  SIM_IO,
};
enum CtxtClockType {
  WALL,
  SIM_CLOCK,
};
enum CtxtLoggerType {
  FILE_LOGGER,
  STDERR_LOGGER,
};
enum CtxtDeadlineTrackerType {
  MIN_HEAP,
};
enum CtxtRandomType { SEEDED, NONE };
enum ContextType {
  NODE,
  SERVER,
  SIMULATOR,
};
struct ContextSettings {
  static constexpr uint64_t clock_tick_ms = 0;
  static constexpr uint64_t max_coroutine_amount = 0;
  static constexpr uint64_t max_coroutine_size = 0;
  static constexpr uint64_t max_coroutine_generator_size = 0;
  static constexpr uint64_t max_coroutine_generator_amount = 0;
  static constexpr uint64_t max_buffer_amount = 0;
  static constexpr uint64_t max_buffer_size = 0;
  static constexpr uint64_t max_deadlines = 0;
  static constexpr uint64_t max_queue_depth = 0;
  static constexpr uint64_t max_ready_queue = 0;
  static constexpr uint64_t max_staging_queue = 0;
  static constexpr uint64_t max_total_size = 0;
};
template <typename Setting> struct ContextConfig {
  ContextType ctx_type;
  CtxtIOType io_type;
  CtxtClockType clock_type;
  CtxtLoggerType logger_type;
  CtxtDeadlineTrackerType deadline_tracker_type;
  CtxtRandomType random_type;
  RandomInterval random_intervals[RANDOM_TYPE_AMOUNT];
  Setting settings;
};

struct ProgramContext {
  LoggerInterface *logger = nullptr;
  ClockInterface *clock = nullptr;
  EventLoopInterface *loop = nullptr;
  AllocatorInterface *frame_allocator = nullptr;
  AllocatorInterface *buffer_allocator = nullptr;
  DeadlineStorageInterface *deadline_tracker = nullptr;
  IOInterface *io = nullptr;
  MetricsInterface *metrics = nullptr;
  RandomInterface *random = nullptr;
};

#endif
