#include "common/server/common.h"
#include "common/io/io.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/deadline_storage/min_heap_storage.h"
#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include "general/interfaces/storage/allocators/bucket_allocator.h"
#include "general/interfaces/storage/queue.h"

void init_globals(AllocatorInterface *allocator) {
  // Init frame allocator
  auto frame_buffer =
      (Bucket<300> *)allocator->allocate(sizeof(Bucket<300>) * 20).value();
  auto frame_allocator_ptr = (BucketAllocator<20, 300> *)allocator
                                 ->allocate(sizeof(BucketAllocator<20, 300>))
                                 .value();
  new (frame_allocator_ptr) BucketAllocator<20, 300>(frame_buffer);
  program_coroutine_frame_allocator = frame_allocator_ptr;

  // Init buffer allocator
  auto buffer_ptr =
      (Bucket<1024> *)allocator->allocate(sizeof(Bucket<1024>) * 20).value();
  auto buffer_allocator_ptr = (BucketAllocator<20, 1024> *)allocator
                                  ->allocate(sizeof(BucketAllocator<20, 1024>))
                                  .value();
  new (buffer_allocator_ptr) BucketAllocator<20, 1024>(buffer_ptr);
  program_buffer_allocator = buffer_allocator_ptr;

  // Init event loop
  auto ready_queue_ptr =
      (Queue<std::coroutine_handle<>, 20> *)allocator
          ->allocate(sizeof(Queue<std::coroutine_handle<>, 20>))
          .value();
  new (ready_queue_ptr) Queue<std::coroutine_handle<>, 20>();

  auto staging_queue_ptr =
      (Queue<std::coroutine_handle<>, 20> *)allocator
          ->allocate(sizeof(Queue<std::coroutine_handle<>, 20>))
          .value();

  new (staging_queue_ptr) Queue<std::coroutine_handle<>, 20>();
  auto gen_buffer_ptr =
      (Bucket<30> *)allocator->allocate(sizeof(Bucket<30>) * 20).value();
  auto gen_allocator_ptr = (BucketAllocator<20, 30> *)allocator
                               ->allocate(sizeof(BucketAllocator<20, 30>))
                               .value();
  new (gen_allocator_ptr) BucketAllocator<20, 30>(gen_buffer_ptr);
  auto event_loop_ptr =
      (BasicEventLoop *)allocator->allocate(sizeof(BasicEventLoop)).value();
  new (event_loop_ptr)
      BasicEventLoop(ready_queue_ptr, staging_queue_ptr, gen_allocator_ptr);
  program_loop = event_loop_ptr;
  // Init deadline keeper
  auto deadline_buffer_ptr =
      (Bucket<1> *)allocator->allocate(sizeof(Bucket<1>) * 20).value();
  auto deadline_allocator_ptr = (BucketAllocator<20, 1> *)allocator
                                    ->allocate(sizeof(BucketAllocator<20, 1>))
                                    .value();
  new (deadline_allocator_ptr) BucketAllocator<20, 1>(deadline_buffer_ptr);
  auto deadline_storage_ptr =
      (Deadline *)allocator->allocate(sizeof(Deadline) * 20).value();
  auto deadline_keeper_ptr =
      (DeadlineMinHeap *)allocator->allocate(sizeof(DeadlineMinHeap)).value();
  new (deadline_keeper_ptr)
      DeadlineMinHeap(deadline_storage_ptr, 20, deadline_allocator_ptr);
  program_deadline_keeper = deadline_keeper_ptr;

  auto linux_io_ptr = (LinuxIO *)allocator->allocate(sizeof(LinuxIO)).value();
  new (linux_io_ptr) LinuxIO(20);
  program_io = linux_io_ptr;
}
