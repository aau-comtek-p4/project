#include "general/interfaces/utility/clocks/wall_clock.h"
#include "general/common.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/context.h"
#include <cinttypes>
#include <cstdint>

#include <ctime>
uint64_t now_ns(clockid_t clock_id) {
  struct timespec ts;
  clock_gettime(clock_id, &ts);
  return (uint64_t)ts.tv_sec * 1'000'000'000ULL + ts.tv_nsec;
}

WallClock::WallClock(clockid_t clock_id, uint64_t tick_ns)
    : tick_ns(tick_ns), clock_id(clock_id) {
  this->start_ns = now_ns(clock_id);
};
void WallClock::setup() {
  this->future_time = this->rt_now_ns() + this->tick_ns;
}
uint64_t WallClock::rt_now_ns() { return now_ns(this->clock_id); };
uint64_t WallClock::rt_now_ms() { return now_ns(this->clock_id) / NS_PR_MS; };

uint64_t WallClock::rt_since_start_ms() {
  return (this->rt_now_ns() - this->start_ns) / NS_PR_MS;
};
uint64_t WallClock::rt_since_start_ns() {
  return this->rt_now_ns() - this->start_ns;
};
uint64_t WallClock::time_until_tick() {
  return this->future_time - this->rt_now_ns();
};
void WallClock::tick_catchup() { this->tick_count += 1; }
uint64_t WallClock::tick_now() { return this->tick_count; };
uint64_t WallClock::tick() {
  uint64_t start_time = this->rt_now_ns();
  uint64_t missed_ticks = 0;
  if (start_time > this->future_time) {
    uint64_t missed_ns = start_time - this->future_time;
    missed_ticks = missed_ns / this->tick_ns;
    if (missed_ns > this->tick_ns * MISSED_TICK_WARNING_THRESHOLD) {
      program_ctxt->logger->log_warning(
          CLOCK_TAG,
          "Missed tick threshold surpassed, missed ns: [%" PRIu64
          "], threshold: [%" PRIu64 "]",
          missed_ns, (uint64_t)(this->tick_ns * MISSED_TICK_WARNING_THRESHOLD));
      program_ctxt->metrics->document_metric(MetricType::TICK_MISS);
    }
  }
  while (start_time < this->future_time) {
    start_time = this->rt_now_ns();
  }
  this->tick_count += 1;
  if (missed_ticks > MAX_MISSED_TICK) {
    program_ctxt->logger->log_warning(
        CLOCK_TAG,
        "Max missed tick threshold reached, missed ticks: [%" PRIu64 "]",
        missed_ticks);
    this->future_time = start_time + this->tick_ns;
  } else {
    this->future_time += (missed_ticks + 1) * this->tick_ns;
  }
  return missed_ticks;
};
uint64_t WallClock::ms_pr_tick() { return this->tick_ns / NS_PR_MS; }
uint64_t WallClock::ms_to_tick(uint64_t time_ms) {
  return time_ms / this->ms_pr_tick();
}
