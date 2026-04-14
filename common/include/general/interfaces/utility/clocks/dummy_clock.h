#ifndef DUMMY_CLOCK_H
#define DUMMY_CLOCK_H
#include "general/interfaces/utility/clock.h"
#include <cstdint>
#include <sys/types.h>

class DummyClock : public ClockInterface {
public:
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
