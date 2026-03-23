#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include "general/common.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include <cstdint>
#include <utility>

BasicEventLoop::BasicEventLoop(
    QueueInterface<std::coroutine_handle<>> *ready_queue,
    QueueInterface<std::coroutine_handle<>> *staging_queue,
    AllocatorInterface *coroutine_generator_allocator)
    : ready_queue(ready_queue), staging_queue(staging_queue),
      coroutine_generator_allocator(coroutine_generator_allocator) {}

std::expected<void *, int> BasicEventLoop::allocate(size_t n) {
  auto res = this->coroutine_generator_allocator->allocate(n);
  if (res.has_value()) {
    return res.value();
  }
  program_logger->log_err(
      EVENT_LOOP_ERR_TAG,
      "Failed to allocate space for coroutine generator, received err: [%s]",
      custom_strerror(res.error()));
  return std::unexpected(res.error());
};

std::expected<void, int> BasicEventLoop::enque(std::coroutine_handle<> handle) {
  auto res = this->ready_queue->enque(std::move(handle));
  if (res.has_value()) {
    return {};
  }
  program_logger->log_err(
      EVENT_LOOP_ERR_TAG,
      "Failed to enque co routine handle to ready queue, received err: [%s]",
      custom_strerror(res.error()));
  return std::unexpected(res.error());
};
std::expected<void, int>
BasicEventLoop::enque_staging(std::coroutine_handle<> handle) {
  auto res = this->staging_queue->enque(std::move(handle));
  if (res.has_value()) {
    return {};
  }
  program_logger->log_err(
      EVENT_LOOP_ERR_TAG,
      "Failed to enque co routine handle to staging queue, received err: [%s]",
      custom_strerror(res.error()));
  return std::unexpected(res.error());
};

std::expected<void, int>
BasicEventLoop::set_future(std::coroutine_handle<> handle,
                           uint64_t future_tick) {

  auto res =
      program_deadline_keeper->add_deadline(std::move(handle), future_tick);
  if (res.has_value()) {
    return {};
  }
  program_logger->log_err(
      EVENT_LOOP_ERR_TAG,
      "Failed to enque future co routine handle to wheel, received err: [%s]",
      custom_strerror(res.error()));
  return std::unexpected(res.error());
}

std::expected<void, int> BasicEventLoop::step() {

  std::swap(this->ready_queue, this->staging_queue);
  auto get_head_handle_res = this->ready_queue->deque();
  while (get_head_handle_res.has_value()) {
    std::coroutine_handle<> *handler = get_head_handle_res.value();
    if (handler) {
      if (!handler->done()) {
        handler->resume();
      }
    } else {

      program_logger->log_debug(EVENT_LOOP_TAG, "NULL handle");
    }

    get_head_handle_res = this->ready_queue->deque();
  }

  auto res = program_deadline_keeper->enforce_deadlines();
  if (!res.has_value()) {
    program_logger->log_err(EVENT_LOOP_ERR_TAG,
                            "Failed to enforce deadlines, error: [%s]",
                            custom_strerror(res.error()));
  }
  program_io->submit();
  program_io->process_cqe(program_clock->time_untill_futute() * 0.5);

  // program_io->process_cqe(0);
  uint64_t tick_start = program_clock->spin_untill_future();
  program_clock->tick();
  program_clock->set_future_tick(tick_start);
  return {};
}

std::expected<void, int> BasicEventLoop::run() {
  uint64_t tick_start = program_clock->spin_untill_future();
  program_clock->set_future_tick(tick_start);
  while (true) {
    auto res = this->step();
    if (!res.has_value()) {
      program_logger->log_err(EVENT_LOOP_ERR_TAG,
                              "Failed to take event loop step, error: [%s]",
                              custom_strerror(res.error()));
    }
  }
};
