#include "common/simulation/sim_clock.h"
#include "general/common.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"

SimClock::SimClock() : tick_ns(1) {};
void SimClock::tick() { this->tick_count += 1; };
uint64_t SimClock::rt_now() { return this->tick_count; };
uint64_t SimClock::rt_since_start() { return this->tick_count; };
uint64_t SimClock::tick_now() { return this->tick_count; };
void SimClock::set_future_tick(uint64_t current_time) {};
void SimClock::set_future_time(uint64_t time_until) {};
uint64_t SimClock::time_untill_futute() { return 0; };
uint64_t SimClock::spin_untill_future() { return this->tick_count; };

TimeStamp SimClock::format_time() {
  uint64_t time_ns = this->rt_since_start();
  uint64_t time_ms = time_ns / NS_PR_MS;
  uint64_t time_s = time_ms / MS_PR_S;
  uint64_t time_m = time_s / S_PR_M;
  uint64_t time_h = time_m / M_PR_H;

  TimeStamp time_stamp{
      .time_ns = time_ns,
      .time_ms = time_ns,
      .time_s = time_s,
      .time_m = time_m,
      .time_h = time_h,
  };

  return time_stamp;
}

uint64_t SimClock::get_time_pr_tick() { return this->tick_ns; }
