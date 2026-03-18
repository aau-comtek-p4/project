#ifndef CLOCK_INTERFACE_H
#define CLOCK_INTERFACE_H

#include <cstdint>

#define NS_PR_MS 1000000
#define MS_PR_S 1000
#define S_PR_M 60
#define M_PR_H 60

struct TimeStamp {
  uint64_t time_ns;
  uint64_t time_ms;
  uint64_t time_s;
  uint64_t time_m;
  uint64_t time_h;
};

class ClockInterface {
public:
  virtual void tick() = 0;
  virtual uint64_t rt_now() = 0;
  virtual uint64_t tick_now() = 0;
  virtual void set_future_tick() = 0;
  virtual void set_future_time(uint64_t time_until) = 0;
  virtual uint64_t time_untill_futute() = 0;
  virtual uint64_t spin_untill_future() = 0;

  virtual TimeStamp format_time() = 0;
};

#endif
