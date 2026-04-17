#ifndef CLOCK_INTERFACE_H
#define CLOCK_INTERFACE_H

#include <cstdint>

#define NS_PR_MS 1000000
#define MS_PR_S 1000
#define S_PR_M 60
#define M_PR_H 60

#define CLOCK_TAG "CLOCK"
#define CLOCK_ERROR_TAG "CLOCK ERROR"

class ClockInterface {
public:
  virtual uint64_t rt_now_ns() = 0;
  virtual uint64_t rt_now_ms() = 0;
  virtual uint64_t rt_since_start_ms() = 0;
  virtual uint64_t rt_since_start_ns() = 0;
};

#endif
