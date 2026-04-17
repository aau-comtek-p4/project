#ifndef CONTEXT_H
#define CONTEXT_H
#include <cstdint>
#include <general/common.h>
#include <general/misc/names.h>
#define CONTEXT_TAG "CONTEXT"
#define CONTEXT_ERROR_TAG "CONTEXT ERROR"
enum CtxtClockType {
  WALL,
  SIM_CLOCK,
};
enum CtxtLoggerType {
  FILE_LOGGER,
  FWRITE_LOGGER,
  DUMMY_LOGGER,
  ESP_LOGGER,
};
enum CtxtLoggerSerializer {
  JSON_SERIALIZER,
};
enum CtxtDeadlineTrackerType {
  MIN_HEAP,
};
enum CtxtMetricType { STANDARD_METRIC };
enum CtxtRandomType { SEEDED, NONE };
enum ContextType {
  NODE,
  SERVER,
  SIMULATOR,
};
struct ContextSettings {
  static constexpr uint64_t max_coroutine_amount = 0;
  static constexpr uint64_t max_coroutine_size = 0;
  static constexpr uint64_t max_buffer_amount = 0;
  static constexpr uint64_t max_buffer_size = 0;
  static constexpr uint64_t max_deadlines = 0;
  static constexpr uint64_t max_queue_depth = 0;
  static constexpr uint64_t max_loop_queue = 0;
  static constexpr uint64_t max_total_size = 0;
  static constexpr uint64_t max_io_transport_size = 0;
  static constexpr uint64_t max_log_amount = 0;
  static constexpr uint64_t random_seed = 0;
};

template <typename Setting> struct ContextConfig {
  ContextType ctx_type;
  CtxtClockType clock_type;
  CtxtLoggerType logger_type;
  CtxtLoggerSerializer log_serializer_type;
  CtxtDeadlineTrackerType deadline_tracker_type;
  CtxtRandomType random_type;
  CtxtMetricType metric_type;
  Setting settings;
};

struct ProgramContext {
  NameLookupInterface *name_lookup;
  LoggerInterface *logger = nullptr;
  ClockInterface *clock = nullptr;
  EventLoopInterface *loop = nullptr;
  AllocatorInterface *frame_allocator = nullptr;
  AllocatorInterface *buffer_allocator = nullptr;
  DeadlineStorageInterface *deadline_tracker = nullptr;
  IOHandler *io = nullptr;
  MetricsInterface *metrics = nullptr;
  RandomInterface *random = nullptr;
};
#endif
