#include "general/common.h"
#include "common.h"
#include "common/io/io.h"
#include "common/server/common.h"
#include "common/utility/metric.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/deadline_storage/min_heap_storage.h"
#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/storage/allocators/bucket_allocator.h"
#include "general/interfaces/storage/queue.h"
#include <coroutine>
#include <cstddef>
#include <cstring>

size_t server_innit_frame_allocator(ProgramContext *ctxt,
                                    AllocatorInterface *allocator);
size_t server_innit_bucket_allocator(ProgramContext *ctxt,
                                     AllocatorInterface *allocator);
size_t server_innit_event_loop(ProgramContext *ctxt,
                               AllocatorInterface *allocator);
size_t server_innit_deadline(ProgramContext *ctxt,
                             AllocatorInterface *allocator);
size_t server_innit_io(ProgramContext *ctxt, AllocatorInterface *allocator);
size_t server_innit_metrics(ProgramContext *ctxt,
                            AllocatorInterface *allocator);
void server_init_ctxt(ProgramContext *ctxt, AllocatorInterface *allocator) {

  size_t total_expected = 0;
  const size_t expected_frame_allocator_size =
      server_innit_frame_allocator(ctxt, allocator);
  total_expected += expected_frame_allocator_size;
  const size_t actual_frame_allocator_size = allocator->amount_allocated;
  ctxt->logger->log_info(
      SERVER_TAG,
      "Frame allocator initialized, expected size: [%lu], actual size: [%lu]",
      expected_frame_allocator_size, actual_frame_allocator_size);
  const size_t expected_bucket_allocator_size =
      server_innit_bucket_allocator(ctxt, allocator);

  total_expected += expected_bucket_allocator_size;
  const size_t actual_bucket_allocator_size =
      allocator->amount_allocated - actual_frame_allocator_size;
  ctxt->logger->log_info(
      SERVER_TAG,
      "Bucket allocator initialized, expected size: [%lu], actual size: [%lu]",
      expected_bucket_allocator_size, actual_bucket_allocator_size);
  const size_t expected_event_loop_size =
      server_innit_event_loop(ctxt, allocator);

  total_expected += expected_event_loop_size;
  const size_t actual_event_loop_size = allocator->amount_allocated -
                                        actual_bucket_allocator_size -
                                        actual_frame_allocator_size;
  ctxt->logger->log_info(
      SERVER_TAG,
      "Event loop initialized, expected size: [%lu], actual size: [%lu]",
      expected_event_loop_size, actual_event_loop_size);
  const size_t expected_deadline_size = server_innit_deadline(ctxt, allocator);

  total_expected += expected_deadline_size;
  const size_t actual_deadline_size =
      allocator->amount_allocated - actual_event_loop_size -
      actual_bucket_allocator_size - actual_frame_allocator_size;

  ctxt->logger->log_info(
      SERVER_TAG,
      "Deadline initialized, expected size: [%lu], actual size: [%lu]",
      expected_deadline_size, actual_deadline_size);
  const size_t expected_io_size = server_innit_io(ctxt, allocator);

  total_expected += expected_io_size;
  const size_t actual_io_size =
      allocator->amount_allocated - actual_deadline_size -
      actual_frame_allocator_size - actual_event_loop_size -
      actual_bucket_allocator_size;

  ctxt->logger->log_info(
      SERVER_TAG, "IO initialized, expected size: [%lu], actual size: [%lu]",
      expected_io_size, actual_io_size);
  const size_t expected_metric_size = server_innit_metrics(ctxt, allocator);
  total_expected += expected_metric_size;
  const size_t actual_metric_size =
      allocator->amount_allocated - actual_io_size - actual_deadline_size -
      actual_frame_allocator_size - actual_event_loop_size -
      actual_bucket_allocator_size;

  ctxt->logger->log_info(
      SERVER_TAG,
      "Metric initialized, expected size: [%lu], actual size: [%lu]",
      expected_metric_size, actual_metric_size);

  ctxt->logger->log_info(SERVER_TAG,
                         "Allocated globals, expected: [%lu], actual: [%lu]",
                         total_expected, allocator->amount_allocated);
}

size_t server_innit_frame_allocator(ProgramContext *ctxt,
                                    AllocatorInterface *allocator) {
  using frame_allocator_type =
      BucketAllocator<MAX_COROUTINE_AMOUNT, MAX_COROUTINE_SIZE>;
  const size_t frame_buffer_size =
      sizeof(Bucket<MAX_COROUTINE_SIZE>) * MAX_COROUTINE_AMOUNT;
  const size_t frame_allocator_size = sizeof(frame_allocator_type);
  // Init frame allocator
  auto frame_buffer =
      (Bucket<MAX_COROUTINE_SIZE> *)allocator->allocate(frame_buffer_size)
          .value();
  auto frame_allocator_ptr =
      (frame_allocator_type *)allocator->allocate(frame_allocator_size).value();
  new (frame_allocator_ptr) frame_allocator_type(frame_buffer);
  ctxt->frame_allocator = frame_allocator_ptr;
  return frame_buffer_size + frame_allocator_size;
}
size_t server_innit_bucket_allocator(ProgramContext *ctxt,
                                     AllocatorInterface *allocator) {
  using buffer_allocator_type =
      BucketAllocator<MAX_BUFFER_AMOUNT, MAX_BUFFER_SIZE>;

  const size_t buffer_buffer_size =
      sizeof(Bucket<MAX_BUFFER_SIZE>) * MAX_BUFFER_AMOUNT;
  const size_t buffer_allocator_size = sizeof(buffer_allocator_type);

  // Init buffer allocator
  auto buffer_ptr =
      (Bucket<MAX_BUFFER_SIZE> *)allocator->allocate(buffer_buffer_size)
          .value();
  auto buffer_allocator_ptr =
      (buffer_allocator_type *)allocator->allocate(buffer_allocator_size)
          .value();
  new (buffer_allocator_ptr) buffer_allocator_type(buffer_ptr);
  ctxt->buffer_allocator = buffer_allocator_ptr;
  return buffer_buffer_size + buffer_allocator_size;
}
size_t server_innit_event_loop(ProgramContext *ctxt,
                               AllocatorInterface *allocator) {
  using coroutine_generator_allocator_type =
      BucketAllocator<MAX_COROUTINE_GENERATOR_AMOUNT,
                      MAX_COROUTINE_GENERATOR_SIZE>;
  using event_loop_type = BasicEventLoop;
  const size_t ready_queue_size =
      sizeof(Queue<std::coroutine_handle<>, MAX_READY_QUEUE>);
  const size_t staging_queue_size =
      sizeof(Queue<std::coroutine_handle<>, MAX_STAGING_QUEUE>);
  const size_t generator_buffer_size =
      sizeof(Bucket<MAX_COROUTINE_GENERATOR_SIZE>) *
      MAX_COROUTINE_GENERATOR_AMOUNT;
  const size_t coroutine_generator_allocator_size =
      sizeof(coroutine_generator_allocator_type);
  const size_t event_loop_size = sizeof(event_loop_type);

  // Init event loop
  auto ready_queue_ptr =
      (Queue<std::coroutine_handle<>, MAX_READY_QUEUE> *)allocator
          ->allocate(ready_queue_size)
          .value();
  new (ready_queue_ptr) Queue<std::coroutine_handle<>, MAX_READY_QUEUE>();

  auto staging_queue_ptr =
      (Queue<std::coroutine_handle<>, MAX_STAGING_QUEUE> *)allocator
          ->allocate(staging_queue_size)
          .value();

  new (staging_queue_ptr) Queue<std::coroutine_handle<>, MAX_STAGING_QUEUE>();
  auto gen_buffer_ptr = (Bucket<MAX_COROUTINE_GENERATOR_SIZE> *)allocator
                            ->allocate(generator_buffer_size)
                            .value();
  auto gen_allocator_ptr = (coroutine_generator_allocator_type *)allocator
                               ->allocate(coroutine_generator_allocator_size)
                               .value();
  new (gen_allocator_ptr) coroutine_generator_allocator_type(gen_buffer_ptr);
  auto event_loop_ptr =
      (event_loop_type *)allocator->allocate(event_loop_size).value();
  new (event_loop_ptr)
      BasicEventLoop(ready_queue_ptr, staging_queue_ptr, gen_allocator_ptr);
  ctxt->loop = event_loop_ptr;
  return ready_queue_size + staging_queue_size + generator_buffer_size +
         coroutine_generator_allocator_size + event_loop_size;
}

size_t server_innit_deadline(ProgramContext *ctxt,
                             AllocatorInterface *allocator) {

  const size_t deadline_size = sizeof(DeadlineIndexKeeper);
  using deadline_keeper_type = DeadlineMinHeap;
  using deadline_allocator_type = BucketAllocator<MAX_DEADLINES, deadline_size>;
  const size_t deadline_buffer_size =
      sizeof(Bucket<deadline_size>) * MAX_DEADLINES;
  const size_t deadline_allocator_size = sizeof(deadline_allocator_type);

  const size_t deadline_storage_size = sizeof(Deadline) * MAX_DEADLINES;
  const size_t deadline_keeper_size = sizeof(deadline_keeper_type);

  // Init deadline keeper
  auto deadline_buffer_ptr =
      (Bucket<deadline_size> *)allocator->allocate(deadline_buffer_size)
          .value();
  auto deadline_allocator_ptr =
      (deadline_allocator_type *)allocator->allocate(deadline_allocator_size)
          .value();
  new (deadline_allocator_ptr) deadline_allocator_type(deadline_buffer_ptr);
  auto deadline_storage_ptr =
      (Deadline *)allocator->allocate(deadline_storage_size).value();
  auto deadline_keeper_ptr =
      (deadline_keeper_type *)allocator->allocate(deadline_keeper_size).value();
  new (deadline_keeper_ptr) deadline_keeper_type(
      deadline_storage_ptr, MAX_DEADLINES, deadline_allocator_ptr);
  ctxt->deadline_tracker = deadline_keeper_ptr;
  return deadline_buffer_size + deadline_allocator_size +
         deadline_storage_size + deadline_keeper_size;
}
size_t server_innit_io(ProgramContext *ctxt, AllocatorInterface *allocator) {
  using io_type = LinuxIO;

  const size_t io_size = sizeof(io_type);

  auto linux_io_ptr = (io_type *)allocator->allocate(io_size).value();
  new (linux_io_ptr) io_type(MAX_QUEUE_DEPTH);
  ctxt->io = linux_io_ptr;

  return io_size;
}
size_t server_innit_metrics(ProgramContext *ctxt,
                            AllocatorInterface *allocator) {
  using metric_type = LinuxMetric;
  const size_t metric_size = sizeof(metric_type);
  auto metric_ptr = (metric_type *)allocator->allocate(metric_size).value();
  new (metric_ptr) metric_type();
  ctxt->metrics = metric_ptr;
  return metric_size;
}
