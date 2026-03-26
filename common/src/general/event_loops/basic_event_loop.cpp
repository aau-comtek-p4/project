#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include "general/common.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>

BasicEventLoop::BasicEventLoop(
    QueueInterface<std::coroutine_handle<>> *ready_queue,
    QueueInterface<std::coroutine_handle<>> *staging_queue,
    AllocatorInterface *coroutine_generator_allocator)
    : ready_queue(ready_queue), staging_queue(staging_queue),
      coroutine_generator_allocator(coroutine_generator_allocator),
      running(true) {}

std::expected<void *, ErrorWrapper> BasicEventLoop::allocate(size_t n) {
  auto res = this->coroutine_generator_allocator->allocate(n);
  if (res.has_value()) {
    return res.value();
  }
  program_ctxt->logger->log_err(
      EVENT_LOOP_ERR_TAG,
      "Failed to allocate space for coroutine generator, received err: [%s]",
      custom_strerror(res.error()));
  return std::unexpected(res.error());
};

std::expected<void, ErrorWrapper> BasicEventLoop::free(void *ptr) {
  auto res = this->coroutine_generator_allocator->free(ptr);
  if (res.has_value()) {
    return {};
  }
  program_ctxt->logger->log_err(
      EVENT_LOOP_ERR_TAG,
      "Failed to free co routine generator, received err: [%s]",
      custom_strerror(res.error()));
  return std::unexpected(res.error());
};

std::expected<void, ErrorWrapper>
BasicEventLoop::enque(std::coroutine_handle<> handle) {
  auto res = this->ready_queue->enque(std::move(handle));
  if (res.has_value()) {
    return {};
  }
  program_ctxt->logger->log_err(
      EVENT_LOOP_ERR_TAG,
      "Failed to enque co routine handle to ready queue, received err: [%s]",
      custom_strerror(res.error()));
  return std::unexpected(res.error());
};
std::expected<void, ErrorWrapper>
BasicEventLoop::enque_staging(std::coroutine_handle<> handle) {
  auto res = this->staging_queue->enque(std::move(handle));
  if (res.has_value()) {
    return {};
  }
  program_ctxt->logger->log_err(
      EVENT_LOOP_ERR_TAG,
      "Failed to enque co routine handle to staging queue, received err: [%s]",
      custom_strerror(res.error()));
  return std::unexpected(res.error());
};

std::expected<void, ErrorWrapper>
BasicEventLoop::set_future(std::coroutine_handle<> handle,
                           uint64_t future_tick) {

  auto res = program_ctxt->deadline_tracker->add_deadline(std::move(handle),
                                                          nullptr, future_tick);
  if (res.has_value()) {
    return {};
  }
  program_ctxt->logger->log_err(
      EVENT_LOOP_ERR_TAG,
      "Failed to enque future co routine handle to wheel, received err: [%s]",
      custom_strerror(res.error()));
  return std::unexpected(res.error());
}

std::expected<void, ErrorWrapper>
BasicEventLoop::run_step(uint64_t cqe_timeout) {
  std::swap(this->ready_queue, this->staging_queue);
  auto get_head_handle_res = this->ready_queue->deque();
  while (get_head_handle_res.has_value()) {
    std::coroutine_handle<> *handler = get_head_handle_res.value();
    if (handler) {
      if (!handler->done()) {
        handler->resume();
      }
    } else {

      program_ctxt->logger->log_debug(EVENT_LOOP_TAG, "NULL handle");
    }

    get_head_handle_res = this->ready_queue->deque();
  }

  auto res = program_ctxt->deadline_tracker->enforce_deadlines();
  if (!res.has_value()) {
    program_ctxt->logger->log_err(EVENT_LOOP_ERR_TAG,
                                  "Failed to enforce deadlines, error: [%s]",
                                  custom_strerror(res.error()));
  }
  program_ctxt->io->submit();
  program_ctxt->io->process_cqe(cqe_timeout);
  return {};
}

std::expected<void, ErrorWrapper> BasicEventLoop::step() {

  auto res = this->run_step(program_ctxt->clock->time_until_tick());

  uint64_t missed_ticks = program_ctxt->clock->tick();

  uint64_t remaining_steps = std::min(missed_ticks, (uint64_t)MAX_MISSED_TICK);

  for (size_t i = 0; i < remaining_steps; i++) {
    res = this->run_step(0);
    program_ctxt->clock->tick_catchup();
  }
  return {};
}

std::expected<void, ErrorWrapper> BasicEventLoop::run() {
  while (this->running) {
    auto res = this->step();
    if (!res.has_value()) {
      program_ctxt->logger->log_err(
          EVENT_LOOP_ERR_TAG, "Failed to take event loop step, error: [%s]",
          custom_strerror(res.error()));
    }
  }
  return {};
};
void BasicEventLoop::stop() { this->running = false; }
