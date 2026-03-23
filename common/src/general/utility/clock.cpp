#include "general/interfaces/utility/clock.h"
#include "general/common.h"
#include <cstdint>
uint64_t ms_to_tick(uint64_t time_ms) {
  return time_ms * NS_PR_MS / (program_clock->get_time_pr_tick());
}
