#include "general/awaiters/sleep_for.h"

SkipAwaiter sleep_for(uint64_t timeout_tick) {
  return SkipAwaiter(timeout_tick);
}
