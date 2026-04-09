#ifndef CONTEXT_LINUX_H
#define CONTEXT_LINUX_H

#include "common.h"
#include "common/io/io.h"
#include "general/common.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/deadline_storage/min_heap_storage.h"
#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/simulator/random.h"
#include "general/interfaces/simulator/random/null_random.h"
#include "general/interfaces/simulator/random/seeded_random.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/storage/allocators/bucket_allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/clocks/wall_clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/loggers/dummy_logger.h"
#include "general/interfaces/utility/loggers/fwrite_logger.h"
#include "general/interfaces/utility/metrics/standard_metrics.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"
#include <coroutine>
#include <cstdint>
#include <cstdio>

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
    fprintf(stderr, "Sim clock not implemented yet");
    safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
    return;
  }
  case CtxtClockType::WALL: {
    auto wall_clock_ptr =
        (WallClock *)allocator->allocate(sizeof(WallClock)).value();
    new (wall_clock_ptr) WallClock(
        CLOCK_MONOTONIC, ctx_config.settings.clock_tick_ms * NS_PR_MS);

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
    fprintf(stderr, "File logger not implemented yet");
    safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
    return;
  }
  case CtxtLoggerType::FWRITE_LOGGER: {
    auto logger_ptr =
        (FWriteLogger *)allocator->allocate(sizeof(FWriteLogger)).value();
    new (logger_ptr) FWriteLogger();
    ctxt->logger = logger_ptr;
    return;
  }
  case CtxtLoggerType::DUMMY_LOGGER: {
    auto logger_ptr =
        (DummyLogger *)allocator->allocate(sizeof(DummyLogger)).value();
    new (logger_ptr) DummyLogger();
    ctxt->logger = logger_ptr;
    return;
  }
  case CtxtLoggerType::ESP_LOGGER: {
    fprintf(stderr, "ESP logger not allowed on linux");
    safe_shutdown(ErrorWrapper{.tag = ErrorWrapper::CUSTOM, .error = 1});
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

    auto deadline_buffer =
        (Deadline *)allocator
            ->allocate(sizeof(Deadline) * ctx_config.settings.max_deadlines)
            .value();
    auto deadline_tracker_ptr =
        (DeadlineMinHeap *)allocator->allocate(sizeof(DeadlineMinHeap)).value();
    new (deadline_tracker_ptr)
        DeadlineMinHeap(deadline_buffer, ctx_config.settings.max_deadlines);
    ctxt->deadline_tracker = deadline_tracker_ptr;
    break;
  }
}

template <typename Config>
void innit_io(ProgramContext *ctxt, Config ctx_config,
              AllocatorInterface *allocator) {
  uint8_t *transport_buffer =
      (uint8_t *)allocator->allocate(ctx_config.settings.max_io_transport_size)
          .value();
  ArenaAllocator *io_transport_allocator =
      (ArenaAllocator *)allocator->allocate(sizeof(ArenaAllocator)).value();
  new (io_transport_allocator) ArenaAllocator(
      transport_buffer, ctx_config.settings.max_io_transport_size);
  IOHandler *io_handler_ptr =
      (IOHandler *)allocator->allocate(sizeof(IOHandler)).value();
  new (io_handler_ptr) IOHandler(io_transport_allocator);
  program_ctxt->io = io_handler_ptr;
  return;
}

template <typename Config>
void innit_metrics(ProgramContext *ctxt, Config ctx_config,
                   AllocatorInterface *allocator) {
  switch (ctx_config.metric_type) {
  case CtxtMetricType::STANDARD_METRIC: {
    StandardMetrics *metric_ptr =
        (StandardMetrics *)allocator->allocate(sizeof(StandardMetrics)).value();
    new (metric_ptr) StandardMetrics();
    ctxt->metrics = metric_ptr;
    return;
  }
  }
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
    new (random_ptr) SeededRandom(RANDOM_SEED);
    program_ctxt->random = random_ptr;
    return;
  }
  }
}

template <typename Config>
void innit_trace(ProgramContext *ctxt, Config ctx_config,
                 AllocatorInterface *allocator) {
  using bucket_type = Bucket<sizeof(Trace)>;
  using allocator_type =
      BucketAllocator<ctx_config.settings.max_trace_amount, sizeof(Trace)>;
  bucket_type *trace_buffer =
      (bucket_type *)allocator
          ->allocate(sizeof(bucket_type) * ctx_config.settings.max_trace_amount)
          .value();

  allocator_type *allocator_ptr =
      (allocator_type *)allocator->allocate(sizeof(allocator_type)).value();
  new (allocator_ptr) allocator_type(trace_buffer);

  TraceHandler *trace_handler =
      (TraceHandler *)allocator->allocate(sizeof(TraceHandler)).value();
  new (trace_handler)
      TraceHandler(allocator_ptr, ctx_config.settings.max_trace_amount);
  program_ctxt->trace_handler = trace_handler;
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
  innit_trace(ctxt, ctx_config, allocator);
  uint64_t actual_trace_size = allocator->amount_allocated - before;
  before += actual_trace_size;
  program_ctxt->logger->log_debug(tag, "Allocated clock, size: [%" PRIu64 "]",
                                  actual_clock_size);
  program_ctxt->logger->log_debug(tag, "Allocated logger, size: [%" PRIu64 "]",
                                  actual_logger_size);
  program_ctxt->logger->log_debug(tag, "Allocated loop, size: [%" PRIu64 "]",
                                  actual_loop_size);
  program_ctxt->logger->log_debug(
      tag, "Allocated buffer allocator, size: [%" PRIu64 "]",
      actual_buffer_alloc_size);
  program_ctxt->logger->log_debug(
      tag, "Allocated frame allocator, size: [%" PRIu64 "]",
      actual_frame_alloc_size);
  program_ctxt->logger->log_debug(
      tag, "Allocated deadline tracker, size: [%" PRIu64 "]",
      actual_deadline_size);
  program_ctxt->logger->log_debug(tag, "Allocated io, size: [%" PRIu64 "]",
                                  actual_io_size);
  program_ctxt->logger->log_debug(tag, "Allocated metrics, size: [%" PRIu64 "]",
                                  actual_metric_size);
  program_ctxt->logger->log_debug(tag, "Allocated random, size: [%" PRIu64 "]",
                                  actual_random_size);
  program_ctxt->logger->log_debug(tag, "Allocated trace, size: [%" PRIu64 "]",
                                  actual_trace_size);

  program_ctxt->logger->log_debug(tag, "Total allocations: [%" PRIu64 "]",
                                  allocator->amount_allocated);
}

#endif
