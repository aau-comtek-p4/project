#include "general/interfaces/simulator/random.h"
#include <cstddef>
#include <cstdint>

#pragma once
struct RandomInterval {
  uint64_t min;
  uint64_t max;
};

class SeededRandom : public RandomInterface {
  uint64_t seed;
  uint64_t call_count;
  uint64_t hash(RandomType random_type, uint64_t ctx);
  RandomInterval intervals[RANDOM_TYPE_AMOUNT];

public:
  SeededRandom(uint64_t seed);
  void add_random_interval(RandomType random_type, uint64_t min_value,
                           uint64_t max_value);

  uint64_t inject_value(RandomType random_type, uint64_t ctx);
  bool inject_fault(RandomType random_type, uint64_t ctx);
};
