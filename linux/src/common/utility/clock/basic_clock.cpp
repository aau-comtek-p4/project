#include "common/utility/clocks/basic_clock.h"
#include "general/common.h"
#include "general/interfaces/utility/clock.h"
#include <algorithm>
#include <cstdint>

#include <ctime>
#include <format>
uint64_t now_ns(clockid_t clock_id) {
  struct timespec ts;
  clock_gettime(clock_id, &ts);
  return (uint64_t)ts.tv_sec * 1'000'000'000ULL + ts.tv_nsec;
}

BasickClock::BasickClock(clockid_t clock_id, uint64_t tick_ns)
    : tick_ns(tick_ns), clock_id(clock_id) {
  this->start_ns = now_ns(clock_id);
};
void BasickClock::tick() { this->tick_count += 1; };
uint64_t BasickClock::rt_now() { return now_ns(this->clock_id); };

uint64_t BasickClock::rt_since_start() {
  return this->rt_now() - this->start_ns;
};
uint64_t BasickClock::tick_now() { return this->tick_count; };
void BasickClock::set_future_tick(uint64_t current_time) {
  uint64_t missed_ticks = 0;
  if (this->future_time == 0) {
    this->future_time = current_time;
  }
  if (current_time > this->future_time) {
    uint64_t missed_ns = current_time - this->future_time;
    missed_ticks = missed_ns / this->tick_ns;

    this->tick_count += missed_ticks;
    if (missed_ns > this->tick_ns * 0.02) {
      tl_logger->log_warning(CLOCK_TAG, "Missed tick by ns: [%lu]", missed_ns);
    }
  }
  this->future_time += (missed_ticks + 1) * this->tick_ns;
};
void BasickClock::set_future_time(uint64_t time_until) {
  this->future_time = this->rt_now() + this->tick_ns;
};
uint64_t BasickClock::time_untill_futute() {
  return this->future_time - this->rt_now();
};
uint64_t BasickClock::spin_untill_future() {
  uint64_t now;
  do {
    now = this->rt_now();
  } while (now < this->future_time);
  return now;
};
TimeStamp BasickClock::format_time() {
  uint64_t time_ns = this->rt_since_start();
  uint64_t time_ms = time_ns / NS_PR_MS;
  uint64_t time_s = time_ms / MS_PR_S;
  uint64_t time_m = time_s / S_PR_M;
  uint64_t time_h = time_m / M_PR_H;

  TimeStamp time_stamp{
      .time_ns = time_ns,
      .time_ms = time_ms,
      .time_s = time_s,
      .time_m = time_m,
      .time_h = time_h,
  };

  return time_stamp;
}

uint64_t BasickClock::get_time_pr_tick() { return this->tick_ns; }
