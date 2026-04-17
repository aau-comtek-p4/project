#ifndef DUMMY_CLOCK_H
#define DUMMY_CLOCK_H
#include "general/interfaces/utility/clock.h"
#include <cstdint>
#include <sys/types.h>

class DummyClock : public ClockInterface {
public:
  uint64_t rt_now_ms() override;
  uint64_t rt_now_ns() override;
  uint64_t rt_since_start_ms() override;
  uint64_t rt_since_start_ns() override;
};

#endif
