#ifndef CLOCK_INTERFACE_H
#define CLOCK_INTERFACE_H

#include <cstdint>

#define SIMULATION 0

#if SIMULATION == 0
#define NS_PR_MS 1000000
#define MS_PR_S 1000
#define S_PR_M 60
#define M_PR_H 60
#else
#define NS_PR_MS 1
#define MS_PR_S 1
#define S_PR_M 1
#define M_PR_H 1
#endif

#define CLOCK_TAG "CLOCK"
#define CLOCK_ERROR_TAG "CLOCK ERROR"

#define MISSED_TICK_WARNING_THRESHOLD 0.50
#define MAX_MISSED_TICK 2
class ClockInterface {
public:
  virtual void setup() = 0;
  virtual uint64_t tick() = 0;
  virtual void tick_catchup() = 0;
  virtual uint64_t rt_now() = 0;
  virtual uint64_t tick_now() = 0;
  virtual uint64_t time_until_tick() = 0;
  virtual uint64_t rt_since_start_ms() = 0;
  virtual uint64_t ms_pr_tick() = 0;
  virtual uint64_t ms_to_tick(uint64_t ms_time) = 0;
};

#endif
