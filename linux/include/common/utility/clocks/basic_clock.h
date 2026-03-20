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
  void tick() override;
  uint64_t rt_now() override;
  uint64_t tick_now() override;
  void set_future_tick(uint64_t) override;
  void set_future_time(uint64_t time_until) override;
  uint64_t time_untill_futute() override;
  uint64_t spin_untill_future() override;
  TimeStamp format_time() override;
  uint64_t rt_since_start() override;
  uint64_t get_time_pr_tick() override;
};

#endif
