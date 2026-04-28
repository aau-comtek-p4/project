#ifndef CONTEXT_INNIT_H
#define CONTEXT_INNIT_H
#include "general/common.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/deadline_storage/min_heap_storage.h"
#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include "general/interfaces/simulator/random/null_random.h"
#include "general/interfaces/simulator/random/seeded_random.h"
#include "general/interfaces/storage/allocators/bucket_allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/clocks/dummy_clock.h"
#include "general/interfaces/utility/clocks/wall_clock.h"
#include "general/interfaces/utility/loggers/dummy_logger.h"
#include "general/interfaces/utility/metrics/standard_metrics.h"
#include "general/misc/context.h"
#include "general/misc/names.h"
#include "general/misc/shutdown.h"
#include <coroutine>
#include <cstdio>

template <typename Config>

void innit_event_loop(ProgramContext *ctxt, ContextConfig<Config> ctx_config,
                      AllocatorInterface *allocator) {

  using queue_type =
      Queue<std::coroutine_handle<>, ctx_config.settings.max_loop_queue>;
  using event_loop_type = BasicEventLoop;
  auto queue_ptr =
      (queue_type *)allocator->allocate(sizeof(queue_type)).value();

  new (queue_ptr) queue_type(NAME_READY_QUEUE);
  auto event_loop_ptr =
      (event_loop_type *)allocator->allocate(sizeof(event_loop_type)).value();

  new (event_loop_ptr) event_loop_type(queue_ptr, 100);
  ctxt->loop = event_loop_ptr;
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
  new (bucket_allocator_ptr)
      allocator_type(NAME_FRAME_ALLOCATOR, bucket_buffer_ptr);
  ctxt->frame_allocator = bucket_allocator_ptr;
}

template <typename Config>
void innit_buffer_allocator(ProgramContext *ctxt,
                            ContextConfig<Config> ctx_config,
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
  new (bucket_allocator_ptr)
      allocator_type(NAME_BUFFER_ALLOCATOR, bucket_buffer_ptr);
  ctxt->buffer_allocator = bucket_allocator_ptr;
}
template <typename Config>
void innit_metrics(ProgramContext *ctxt, Config ctx_config,
                   AllocatorInterface *allocator) {
  StandardMetrics *metric_ptr =
      (StandardMetrics *)allocator->allocate(sizeof(StandardMetrics)).value();
  new (metric_ptr) StandardMetrics();
  ctxt->metrics = metric_ptr;
  return;
}
template <typename Config>
void innit_random(ProgramContext *ctxt, ContextConfig<Config> ctx_config,
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
    new (random_ptr) SeededRandom(ctx_config.settings.random_seed);
    ctxt->random = random_ptr;
    return;
  }
  }
}
template <typename Config>
void innit_names(ProgramContext *ctxt, ContextConfig<Config> ctx_config,
                 AllocatorInterface *allocator) {
  using name_lookup_type = NameLookup<NAME_LEN, NAME_AMOUNT>;
  name_lookup_type *lookup_ptr =
      (name_lookup_type *)allocator->allocate(sizeof(name_lookup_type)).value();
  new (lookup_ptr) name_lookup_type{};
  lookup_ptr->set_name(NAME_NO, "none");
  lookup_ptr->set_name(NAME_BUFFER_ALLOCATOR, "buffer_allocator");
  lookup_ptr->set_name(NAME_FRAME_ALLOCATOR, "frame_allocator");
  lookup_ptr->set_name(NAME_PROGRAM_ALLOCATOR, "program_allocator");
  lookup_ptr->set_name(NAME_TRACE_ALLOCATOR, "trace_allocator");
  lookup_ptr->set_name(NAME_READY_QUEUE, "ready_queue");
  lookup_ptr->set_name(NAME_STAGING_QUEUE, "staging_queue");
  lookup_ptr->set_name(NAME_TIMEOUT_ROUTINE, "timeout_routine");
  lookup_ptr->set_name(NAME_SLEEP_ROUTINE, "sleep_routine");
  lookup_ptr->set_name(NAME_CTRLC_ROUTINE, "ctrlc_routine");
  lookup_ptr->set_name(NAME_LOGGING_QUEUE, "logging_queue");
  lookup_ptr->set_name(NAME_IO_ALLOCATOR, "io_allocator");
  lookup_ptr->set_name(NAME_IO_FILE_CLOSE, "io_file_close");
  lookup_ptr->set_name(NAME_IO_FILE_WRITE, "io_file_write");
  lookup_ptr->set_name(NAME_IO_FILE_READ, "io_file_read");
  lookup_ptr->set_name(NAME_IO_FILE_OPEN, "io_file_open");
  lookup_ptr->set_name(NAME_IO_SERIAL_CLOSE, "io_serial_close");
  lookup_ptr->set_name(NAME_IO_SERIAL_WRITE, "io_serial_write");
  lookup_ptr->set_name(NAME_IO_SERIAL_READ, "io_serial_read");
  lookup_ptr->set_name(NAME_IO_SERIAL_OPEN, "io_serial_open");

  lookup_ptr->set_name(NAME_IO_WIFI_UDP_RECV, "io_wifi_udp_recv");
  lookup_ptr->set_name(NAME_IO_WIFI_UDP_SEND, "io_wifi_udp_send");
  lookup_ptr->set_name(NAME_IO_WIFI_UDP_BIND, "io_wifi_udp_bind");
  lookup_ptr->set_name(NAME_IO_WIFI_UDP_CLOSE, "io_wifi_udp_close");

  ctxt->name_lookup = lookup_ptr;
}
template <typename Config>
void innit_clock(ProgramContext *ctxt, Config ctx_config,
                 AllocatorInterface *allocator) {
  switch (ctx_config.clock_type) {
  case CtxtClockType::SIM_CLOCK: {
    fprintf(stderr, "Sim clock not implemented\n");
    safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
  }
  case CtxtClockType::WALL: {
    auto wall_clock_ptr =
        (WallClock *)allocator->allocate(sizeof(WallClock)).value();
    new (wall_clock_ptr) WallClock(CLOCK_MONOTONIC);
    ctxt->clock = wall_clock_ptr;
    return;
  }
  default: {
    fprintf(stderr, "No clock given\n");
    safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
  }
  }
}
template <typename Config>
void innit_deadline_tracker(ProgramContext *ctxt,
                            ContextConfig<Config> ctx_config,
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
void innit_logger(ProgramContext *ctxt, ContextConfig<Config> ctx_config,
                  AllocatorInterface *allocator);
template <typename Config>
void innit_io(ProgramContext *ctxt, ContextConfig<Config> ctx_config,
              AllocatorInterface *allocator);

template <typename Config>
void innit_ctx(ProgramContext *ctxt, ContextConfig<Config> ctx_config,
               AllocatorInterface *allocator) {
  DummyLogger dummy_logger;
  DummyClock dummy_clock;
  ctxt->clock = &dummy_clock;
  ctxt->logger = &dummy_logger;
  uint64_t before = 0;

  innit_metrics(ctxt, ctx_config, allocator);
  uint64_t actual_metric_size = allocator->amount_allocated - before;
  before += actual_metric_size;

  innit_names(ctxt, ctx_config, allocator);
  uint64_t actual_name_size = allocator->amount_allocated - before;
  before += actual_name_size;

  innit_clock(ctxt, ctx_config, allocator);
  uint64_t actual_clock_size = allocator->amount_allocated - before;
  before += actual_clock_size;

  innit_logger(ctxt, ctx_config, allocator);
  uint64_t actual_logger_size = allocator->amount_allocated - before;
  before += actual_logger_size;

  innit_buffer_allocator(ctxt, ctx_config, allocator);
  uint64_t actual_buffer_alloc_size = allocator->amount_allocated - before;
  before += actual_buffer_alloc_size;

  innit_random(ctxt, ctx_config, allocator);
  uint64_t actual_random_size = allocator->amount_allocated - before;
  before += actual_random_size;

  innit_frame_allocator(ctxt, ctx_config, allocator);
  uint64_t actual_frame_alloc_size = allocator->amount_allocated - before;
  before += actual_frame_alloc_size;

  innit_io(ctxt, ctx_config, allocator);
  uint64_t actual_io_size = allocator->amount_allocated - before;
  before += actual_io_size;

  innit_event_loop(ctxt, ctx_config, allocator);
  uint64_t actual_loop_size = allocator->amount_allocated - before;
  before += actual_loop_size;

  innit_deadline_tracker(ctxt, ctx_config, allocator);
  uint64_t actual_deadline_size = allocator->amount_allocated - before;
  before += actual_deadline_size;
  context_initialized = true;
}

#endif
