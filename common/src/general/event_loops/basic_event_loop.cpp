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
    QueueInterface<std::coroutine_handle<>> *staging_queue)
    : ready_queue(ready_queue), staging_queue(staging_queue), running(true) {}

std::expected<void, ErrorWrapper>
BasicEventLoop::enque(std::coroutine_handle<> handle) {
  auto res = this->ready_queue->enque(std::move(handle));
  if (res.has_value()) {
    return {};
  }
  return std::unexpected(res.error());
};
std::expected<void, ErrorWrapper>
BasicEventLoop::enque_staging(std::coroutine_handle<> handle) {
  auto res = this->staging_queue->enque(std::move(handle));
  if (res.has_value()) {
    return {};
  }
  return std::unexpected(res.error());
};

std::expected<void, ErrorWrapper>
BasicEventLoop::set_future(std::coroutine_handle<> handle,
                           uint64_t future_tick) {

  auto res = program_ctxt->deadline_tracker->add_deadline(std::move(handle),
                                                          future_tick);
  if (res.has_value()) {
    return {};
  }
  return std::unexpected(res.error());
}

std::expected<void, ErrorWrapper>
BasicEventLoop::run_step(uint64_t cqe_timeout, uint64_t log_timeout) {
  std::swap(this->ready_queue, this->staging_queue);
  auto get_head_handle_res = this->ready_queue->deque();
  while (get_head_handle_res.has_value()) {
    std::coroutine_handle<> handler = get_head_handle_res.value();
    if (handler) {
      if (!handler.done()) {
        handler.resume();
      }
    }

    get_head_handle_res = this->ready_queue->deque();
  }

  auto res = program_ctxt->deadline_tracker->enforce_deadlines();
  program_ctxt->io->submit_all();
  program_ctxt->io->process_all(cqe_timeout);
  program_ctxt->logger->submit(log_timeout);
  return {};
}

std::expected<void, ErrorWrapper> BasicEventLoop::step() {

  auto res = this->run_step(program_ctxt->clock->time_until_tick() * 0.4,
                            program_ctxt->clock->time_until_tick() * 0.4);

  uint64_t missed_ticks = program_ctxt->clock->tick();

  uint64_t remaining_steps = std::min(missed_ticks, (uint64_t)MAX_MISSED_TICK);

  for (size_t i = 0; i < remaining_steps; i++) {
    res = this->run_step(0, 0);
    program_ctxt->clock->tick_catchup();
  }
  return {};
}

std::expected<void, ErrorWrapper> BasicEventLoop::run(uint64_t time) {
  while (this->running && program_ctxt->clock->tick_now() < time) {
    auto res = this->step();
  }
  return {};
};

std::expected<void, ErrorWrapper> BasicEventLoop::run() {
  while (this->running) {
    auto res = this->step();
  }
  return {};
};
void BasicEventLoop::stop() { this->running = false; }
