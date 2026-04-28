#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include "general/common.h"
#include "general/interfaces/event_loop/deadline_keeper.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/errors.h"
#include <algorithm>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <utility>

BasicEventLoop::BasicEventLoop(
    QueueInterface<std::coroutine_handle<>> *general_queue, uint64_t max_ops)
    : general_queue(general_queue), max_ops(max_ops) {
  this->running = true;
}

std::expected<void, ErrorWrapper>
BasicEventLoop::enque(std::coroutine_handle<> handle) {
  auto res = this->general_queue->enque(std::move(handle));
  if (res.has_value()) {
    return {};
  }
  return std::unexpected(res.error());
};

std::expected<void, ErrorWrapper>
BasicEventLoop::enque_future(std::coroutine_handle<> handle, uint64_t time_ms) {
  auto res =
      program_ctxt->deadline_tracker->add_deadline(std::move(handle), time_ms);
  if (res.has_value()) {
    return {};
  }
  return std::unexpected(res.error());
}

std::expected<void, ErrorWrapper> BasicEventLoop::run_step() {
  std::expected<std::coroutine_handle<>, ErrorWrapper> get_head_handle_res;
  uint64_t log_budget = 2 * NS_PR_MS;
  uint64_t loop_start = program_ctxt->clock->rt_since_start_ns();
  uint64_t ops = 0;
  auto _ = program_ctxt->deadline_tracker->enforce_deadlines();
  program_ctxt->io->process_all(0);
  do {
    get_head_handle_res = this->general_queue->deque();
    if (!get_head_handle_res.has_value()) {
      break;
    }
    std::coroutine_handle<> handler = get_head_handle_res.value();
    if (handler) {
      if (!handler.done()) {
        handler.resume();
        ops += 1;
      }
    }

  } while (ops < max_ops && this->running);
  program_ctxt->metrics->document_statistics_metric_metric(
      StatMetricType::METRIC_LOOP_OP, ops);

  uint64_t op_end = program_ctxt->clock->rt_since_start_ns();
  program_ctxt->metrics->document_statistics_metric_metric(
      StatMetricType::METRIC_LOOP_OP_TIME, op_end - loop_start);

  program_ctxt->io->submit_all();

  std::optional<uint64_t> smallest_deadline =
      program_ctxt->deadline_tracker->get_smallest_deadline_ns();

  program_ctxt->logger->submit(log_budget);

  uint64_t now = program_ctxt->clock->rt_since_start_ns();
  uint64_t io_timeout = 2 * NS_PR_MS;
  if (smallest_deadline.has_value()) {
    if (smallest_deadline.value() < now) {
      program_ctxt->metrics->document_statistics_metric_metric(
          StatMetricType::METRIC_SURPASSED_DEADLINE,
          op_end - smallest_deadline.value());
      io_timeout = 0;
    } else {
      program_ctxt->metrics->document_statistics_metric_metric(
          StatMetricType::METRIC_TIME_UNTIL_DEADLINE,
          smallest_deadline.value() - now);
      io_timeout = smallest_deadline.value() - now;
    }
  }
  io_timeout = std::min(io_timeout, (uint64_t)(2 * NS_PR_MS));
  program_ctxt->io->process_all(io_timeout);

  uint64_t loop_end = program_ctxt->clock->rt_since_start_ns();
  program_ctxt->metrics->document_statistics_metric_metric(
      StatMetricType::METRIC_LOOP_TIME, loop_end - loop_start);
  return {};
}

std::expected<void, ErrorWrapper> BasicEventLoop::step() {
  auto res = this->run_step();

  return {};
}

std::expected<void, ErrorWrapper> BasicEventLoop::run(uint64_t time_ms) {
  if (time_ms == 0) {
    return this->run();
  }
  while (this->running && program_ctxt->clock->rt_since_start_ms() < time_ms) {
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
