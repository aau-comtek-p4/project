#ifndef SIM_CLOCK_H
#define SIM_CLOCK_H

#include "general/interfaces/utility/clock.h"
#include <cstdint>
class SimClock : public ClockInterface {
private:
  uint64_t tick_count = 0;
  uint64_t tick_ns;

public:
  SimClock();
  void tick() override;
  uint64_t rt_now() override;
  uint64_t tick_now() override;
  void set_future_tick(uint64_t current_time) override;
  void set_future_time(uint64_t time_until) override;
  uint64_t time_untill_futute() override;
  uint64_t spin_untill_future() override;
  TimeStamp format_time() override;
  uint64_t rt_since_start() override;
  uint64_t get_time_pr_tick() override;
};

#endif
