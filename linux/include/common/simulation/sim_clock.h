#ifndef SIM_CLOCK_H
#define SIM_CLOCK_H

#include "general/interfaces/utility/clock.h"
#include <cstdint>
class SimClock : public ClockInterface {
private:
  uint64_t tick_count = 0;
  uint64_t tick_ns;

public:
  SimClock(uint64_t tick_ns);
  void setup() override;
  uint64_t tick() override;
  void tick_catchup() override;
  uint64_t rt_now_ms() override;
  uint64_t rt_now_ns() override;
  uint64_t tick_now() override;
  uint64_t time_until_tick() override;
  uint64_t rt_since_start_ms() override;
  uint64_t rt_since_start_ns() override;
  uint64_t ms_pr_tick() override;
  uint64_t ms_to_tick(uint64_t ms_time) override;
};

#endif
