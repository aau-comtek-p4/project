#ifndef COMMON_LINUX_H
#define COMMON_LINUX_H

#include "common.h"
#include "common/io/io.h"
#include "common/simulation/sim_clock.h"
#include "common/utility/clocks/basic_clock.h"
#include "common/utility/context.h"
#include "common/utility/loggers/fprint_logger.h"
#include "common/utility/metric.h"
#include "general/common.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/deadline_storage/min_heap_storage.h"
#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include "general/interfaces/simulator/random.h"
#include "general/interfaces/simulator/random/null_random.h"
#include "general/interfaces/simulator/random/seeded_random.h"
#include "general/interfaces/storage/allocators/bucket_allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <coroutine>
#include <cstdint>
#include <cstdio>

class DummyLogger : public LoggerInterface {
  __attribute__((format(printf, 3, 4))) void
  log_info(const char *tag, const char *fmt, ...) noexcept override {};

  __attribute__((format(printf, 3, 4))) void
  log_err(const char *tag, const char *fmt, ...) noexcept override {};

  __attribute__((format(printf, 3, 4))) void
  log_warning(const char *tag, const char *fmt, ...) noexcept override {};

  __attribute__((format(printf, 3, 4))) void
  log_debug(const char *tag, const char *fmt, ...) noexcept override {};
};

template <typename Config>
void innit_event_loop(ProgramContext *ctxt, Config ctx_config,
                      AllocatorInterface *allocator) {

  using ready_queue_type =
      Queue<std::coroutine_handle<>, ctx_config.settings.max_ready_queue>;
  using staging_queue_type =
      Queue<std::coroutine_handle<>, ctx_config.settings.max_staging_queue>;
  using bucket_type = Bucket<ctx_config.settings.max_coroutine_generator_size>;
  using generator_allocator_type =
      BucketAllocator<ctx_config.settings.max_coroutine_generator_amount,
                      ctx_config.settings.max_coroutine_generator_size>;
  using event_loop_type = BasicEventLoop;
  auto ready_queue_ptr =
      (ready_queue_type *)allocator->allocate(sizeof(ready_queue_type)).value();

  auto staging_queue_ptr =
      (staging_queue_type *)allocator->allocate(sizeof(staging_queue_type))
          .value();

  new (ready_queue_ptr) ready_queue_type();
  new (staging_queue_ptr) staging_queue_type();
  auto gen_buffer = (bucket_type *)allocator
                        ->allocate(sizeof(bucket_type) *
                                   ctx_config.settings.max_coroutine_amount)
                        .value();

  auto generator_allocator_ptr =
      (generator_allocator_type *)allocator
          ->allocate(sizeof(generator_allocator_type))
          .value();

  new (generator_allocator_ptr) generator_allocator_type(gen_buffer);
  auto event_loop_ptr =
      (event_loop_type *)allocator->allocate(sizeof(event_loop_type)).value();

  new (event_loop_ptr) event_loop_type(ready_queue_ptr, staging_queue_ptr,
                                       generator_allocator_ptr);
  ctxt->loop = event_loop_ptr;
}

template <typename Config>
void innit_clock(ProgramContext *ctxt, Config ctx_config,
                 AllocatorInterface *allocator) {
  switch (ctx_config.clock_type) {
  case CtxtClockType::SIM_CLOCK: {
    auto sim_clock_ptr =
        (SimClock *)allocator->allocate(sizeof(SimClock)).value();
    new (sim_clock_ptr) SimClock(ctx_config.settings.clock_tick_ms);
    ctxt->clock = sim_clock_ptr;
    return;
  }
  case CtxtClockType::WALL: {
    auto wall_clock_ptr =
        (BasickClock *)allocator->allocate(sizeof(BasickClock)).value();
    new (wall_clock_ptr)
        BasickClock(CLOCK_MONOTONIC, ctx_config.settings.clock_tick_ms);
    ctxt->clock = wall_clock_ptr;
    return;
  }
  }
}
template <typename Config>
void innit_logger(ProgramContext *ctxt, Config ctx_config,
                  AllocatorInterface *allocator) {
  switch (ctx_config.logger_type) {
  case CtxtLoggerType::FILE_LOGGER: {
    printf("File logger not implemented\n");
    safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
    return;
  }

  case CtxtLoggerType::STDERR_LOGGER: {
    auto logger_ptr =
        (FPrintLogger *)allocator->allocate(sizeof(FPrintLogger)).value();
    new (logger_ptr) FPrintLogger();
    ctxt->logger = logger_ptr;
    return;
  }
  }
}

template <typename Config>
void innit_frame_allocator(ProgramContext *ctxt, Config ctx_config,
                           AllocatorInterface *allocator) {
  using bucket_type = Bucket<ctx_config.settings.max_coroutine_size>;
  using allocator_type =
      BucketAllocator<ctx_config.settings.max_coroutine_amount,
                      ctx_config.settings.max_coroutine_size>;
  auto bucket_buffer_ptr =
      (bucket_type *)allocator
          ->allocate(sizeof(bucket_type) *
                     ctx_config.settings.max_coroutine_amount)
          .value();
  auto bucket_allocator_ptr =
      (allocator_type *)allocator->allocate(sizeof(allocator_type)).value();
  new (bucket_allocator_ptr) allocator_type(bucket_buffer_ptr);
  ctxt->frame_allocator = bucket_allocator_ptr;
}

template <typename Config>
void innit_buffer_allocator(ProgramContext *ctxt, Config ctx_config,
                            AllocatorInterface *allocator) {
  using bucket_type = Bucket<ctx_config.settings.max_buffer_size>;
  using allocator_type = BucketAllocator<ctx_config.settings.max_buffer_amount,
                                         ctx_config.settings.max_buffer_size>;
  auto bucket_buffer_ptr = (bucket_type *)allocator
                               ->allocate(sizeof(bucket_type) *
                                          ctx_config.settings.max_buffer_amount)
                               .value();
  auto bucket_allocator_ptr =
      (allocator_type *)allocator->allocate(sizeof(allocator_type)).value();
  new (bucket_allocator_ptr) allocator_type(bucket_buffer_ptr);
  ctxt->buffer_allocator = bucket_allocator_ptr;
}

template <typename Config>
void innit_deadline_tracker(ProgramContext *ctxt, Config ctx_config,
                            AllocatorInterface *allocator) {
  switch (ctx_config.deadline_tracker_type) {
  case CtxtDeadlineTrackerType::MIN_HEAP:

    using bucket_type = Bucket<sizeof(DeadlineIndexKeeper)>;
    using allocator_type = BucketAllocator<ctx_config.settings.max_deadlines,
                                           sizeof(DeadlineIndexKeeper)>;
    auto deadline_buffer =
        (Deadline *)allocator
            ->allocate(sizeof(Deadline) * ctx_config.settings.max_deadlines)
            .value();
    auto deadline_index_buffer =
        (bucket_type *)allocator
            ->allocate(sizeof(bucket_type) * ctx_config.settings.max_deadlines)
            .value();
    auto allocator_ptr =
        (allocator_type *)allocator->allocate(sizeof(allocator_type)).value();
    new (allocator_ptr) allocator_type(deadline_index_buffer);
    auto deadline_tracker_ptr =
        (DeadlineMinHeap *)allocator->allocate(sizeof(DeadlineMinHeap)).value();
    new (deadline_tracker_ptr) DeadlineMinHeap(
        deadline_buffer, ctx_config.settings.max_deadlines, allocator_ptr);
    ctxt->deadline_tracker = deadline_tracker_ptr;
    break;
  }
}

template <typename Config>
void innit_io(ProgramContext *ctxt, Config ctx_config,
              AllocatorInterface *allocator) {
  switch (ctx_config.io_type) {
  case CtxtIOType::LINUX: {
    auto io_ptr = (LinuxIO *)allocator->allocate(sizeof(LinuxIO)).value();
    new (io_ptr) LinuxIO(ctx_config.settings.max_queue_depth);
    ctxt->io = io_ptr;
    return;
  }
  case CtxtIOType::SIM_IO:
    printf("Sim io not implemented\n");
    safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
    break;
  }
}

template <typename Config>
void innit_metrics(ProgramContext *ctxt, Config ctx_config,
                   AllocatorInterface *allocator) {
  auto linux_metric_ptr =
      (LinuxMetric *)allocator->allocate(sizeof(LinuxMetric)).value();
  new (linux_metric_ptr) LinuxMetric();
  ctxt->metrics = linux_metric_ptr;
}

template <typename Config>
void innit_random(ProgramContext *ctxt, Config ctx_config,
                  AllocatorInterface *allocator) {
  switch (ctx_config.random_type) {
  case CtxtRandomType::NONE: {
    auto random_ptr =
        (NullRandom *)allocator->allocate(sizeof(NullRandom)).value();
    new (random_ptr) NullRandom();
    ctxt->random = random_ptr;
    return;
  }
  case CtxtRandomType::SEEDED: {
    auto random_ptr =
        (SeededRandom *)allocator->allocate(sizeof(SeededRandom)).value();
    new (random_ptr) SeededRandom(RANDOM_SEED_H);
    for (uint64_t i = 0; i < RANDOM_TYPE_AMOUNT; i++) {
      random_ptr->add_random_interval((RandomType)i,
                                      ctx_config.random_intervals[i].min,
                                      ctx_config.random_intervals[i].max);
    }
    ctxt->random = random_ptr;
  }
  }
}

template <typename Config>
void innit_ctx(ProgramContext *ctxt, Config ctx_config,
               AllocatorInterface *allocator, const char *tag) {
  program_ctxt = ctxt;
  DummyLogger dummy_logger;
  ctxt->logger = &dummy_logger;
  uint64_t before = 0;
  innit_clock(ctxt, ctx_config, allocator);
  uint64_t actual_clock_size = allocator->amount_allocated - before;
  before += actual_clock_size;
  innit_logger(ctxt, ctx_config, allocator);
  uint64_t actual_logger_size = allocator->amount_allocated - before;
  before += actual_logger_size;
  innit_event_loop(ctxt, ctx_config, allocator);
  uint64_t actual_loop_size = allocator->amount_allocated - before;
  before += actual_loop_size;
  innit_buffer_allocator(ctxt, ctx_config, allocator);
  uint64_t actual_buffer_alloc_size = allocator->amount_allocated - before;
  before += actual_buffer_alloc_size;
  innit_frame_allocator(ctxt, ctx_config, allocator);
  uint64_t actual_frame_alloc_size = allocator->amount_allocated - before;
  before += actual_frame_alloc_size;
  innit_deadline_tracker(ctxt, ctx_config, allocator);
  uint64_t actual_deadline_size = allocator->amount_allocated - before;
  before += actual_deadline_size;
  innit_io(ctxt, ctx_config, allocator);
  uint64_t actual_io_size = allocator->amount_allocated - before;
  before += actual_io_size;
  innit_metrics(ctxt, ctx_config, allocator);
  uint64_t actual_metric_size = allocator->amount_allocated - before;
  before += actual_metric_size;
  innit_random(ctxt, ctx_config, allocator);
  uint64_t actual_random_size = allocator->amount_allocated - before;
  before += actual_random_size;
  program_ctxt->logger->log_debug(tag, "Allocated clock, size: [%lu]",
                                  actual_clock_size);
  program_ctxt->logger->log_debug(tag, "Allocated logger, size: [%lu]",
                                  actual_logger_size);
  program_ctxt->logger->log_debug(tag, "Allocated loop, size: [%lu]",
                                  actual_loop_size);
  program_ctxt->logger->log_debug(
      tag, "Allocated buffer allocator, size: [%lu]", actual_buffer_alloc_size);
  program_ctxt->logger->log_debug(tag, "Allocated frame allocator, size: [%lu]",
                                  actual_frame_alloc_size);
  program_ctxt->logger->log_debug(
      tag, "Allocated deadline tracker, size: [%lu]", actual_deadline_size);
  program_ctxt->logger->log_debug(tag, "Allocated io, size: [%lu]",
                                  actual_io_size);
  program_ctxt->logger->log_debug(tag, "Allocated metrics, size: [%lu]",
                                  actual_metric_size);
  program_ctxt->logger->log_debug(tag, "Allocated random, size: [%lu]",
                                  actual_random_size);

  program_ctxt->logger->log_debug(tag, "Total allocations: [%lu]",
                                  allocator->amount_allocated);
}

#endif
