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

WallClock::WallClock(clockid_t clock_id) : clock_id(clock_id) {
  this->start_ns = now_ns(clock_id);
};
uint64_t WallClock::rt_now_ns() { return now_ns(this->clock_id); };
uint64_t WallClock::rt_now_ms() { return now_ns(this->clock_id) / NS_PR_MS; };

uint64_t WallClock::rt_since_start_ms() {
  return (this->rt_now_ns() - this->start_ns) / NS_PR_MS;
};
uint64_t WallClock::rt_since_start_ns() {
  return this->rt_now_ns() - this->start_ns;
};
