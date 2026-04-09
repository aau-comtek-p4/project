#include "common/simulation/sim_clock.h"
#include "general/common.h"
#include "general/interfaces/simulator/random.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/context.h"
#include <cstdint>

SimClock::SimClock(uint64_t tick_ns) : tick_ns(tick_ns) {};
void SimClock::tick_catchup() {

  // program_ctxt->metrics->document_metric(MetricType::REAL_TICK);
  this->tick_count += 1;
}
void SimClock::setup() {}
uint64_t SimClock::tick() {
  uint64_t missed_ticks = 0;
  if (program_ctxt->random->inject_fault(RandomType::MISSED_TICK_CHANCE, 1)) {
    missed_ticks =
        program_ctxt->random->inject_value(RandomType::MISSED_TICK, 1);
    program_ctxt->logger->log_warning(CLOCK_TAG, "Missed ticks: [%lu]",
                                      missed_ticks);
    program_ctxt->metrics->document_metric(MetricType::TICK_MISS);
  }
  program_ctxt->metrics->document_metric(MetricType::REAL_TICK);
  this->tick_count += 1;
  return missed_ticks;
};
uint64_t SimClock::rt_now_ns() { return this->tick_count; };
uint64_t SimClock::rt_now_ms() { return this->tick_count; };
uint64_t SimClock::rt_since_start_ms() { return this->tick_count; };
uint64_t SimClock::rt_since_start_ns() { return this->tick_count; };
uint64_t SimClock::tick_now() { return this->tick_count; };
uint64_t SimClock::time_until_tick() { return 0; }

uint64_t SimClock::ms_pr_tick() { return this->tick_ns; }
uint64_t SimClock::ms_to_tick(uint64_t time_ms) {
  return time_ms / this->ms_pr_tick();
}
