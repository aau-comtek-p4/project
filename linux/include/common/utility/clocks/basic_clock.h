#ifndef BASIC_CLOCK_LINUX_H
#define BASIC_CLOCK_LINUX_H

#include "general/interfaces/utility/clock.h"
#include <bits/types/clockid_t.h>
#include <cstddef>
#include <cstdint>

class BasickClock : public ClockInterface {
private:
  uint64_t tick_count = 0;
  uint64_t future_time = 0;
  uint64_t tick_ns;
  clockid_t clock_id;
  uint64_t start_ns;

public:
  BasickClock(clockid_t clock_id, uint64_t tick_ns);
  void setup() override;
  uint64_t tick() override;
  void tick_catchup() override;
  uint64_t rt_now() override;
  uint64_t tick_now() override;
  uint64_t time_until_tick() override;
  uint64_t rt_since_start_ms() override;
  uint64_t ms_pr_tick() override;
  uint64_t ms_to_tick(uint64_t ms_time) override;
};

#endif
