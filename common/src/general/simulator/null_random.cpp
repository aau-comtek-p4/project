#include "general/interfaces/simulator/random/null_random.h"
#include "general/interfaces/simulator/random.h"
#include <cstdint>

bool NullRandom::inject_fault(RandomType fault_type, uint64_t ctx) { return 0; }
uint64_t NullRandom::inject_value(RandomType fault_type, uint64_t ctx) {
  return 0;
}
