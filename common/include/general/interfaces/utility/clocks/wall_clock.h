#ifndef WALL_CLOCK_H
#define WALL_CLOCK_H

#include "general/interfaces/utility/clock.h"
#include <cstdint>
#include <sys/types.h>

class WallClock : public ClockInterface {
private:
  clockid_t clock_id;
  uint64_t start_ns;

public:
  WallClock(clockid_t clock_id);
  uint64_t rt_now_ms() override;
  uint64_t rt_now_ns() override;
  uint64_t rt_since_start_ms() override;
  uint64_t rt_since_start_ns() override;
};

#endif
