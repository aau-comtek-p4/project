#include "general/interfaces/simulator/random/seeded_random.h"
#include "general/interfaces/simulator/random.h"
#include <cstdint>

SeededRandom::SeededRandom(uint64_t seed) : seed(seed) {};

uint64_t SeededRandom::hash(RandomType random_type, uint64_t ctx) {
  // Splitmix64 random
  uint64_t x = seed ^ (((uint64_t)random_type + 1) * 0x9e3779b97f4a7c15) ^
               ((ctx + 1) * 0x6c62272e07bb0142) ^
               ((call_count++ + 1) * 0xbf58476d1ce4e5b9);
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
  x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
  return x ^ (x >> 31);
}
void SeededRandom::add_random_interval(RandomType random_type, uint64_t min,
                                       uint64_t max) {
  RandomInterval interval{.min = min, .max = max};
  this->intervals[random_type] = interval;
}

uint64_t SeededRandom::inject_value(RandomType random_type, uint64_t ctx) {
  uint64_t h = this->hash(random_type, ctx);
  uint64_t min = this->intervals[random_type].min;
  uint64_t max = this->intervals[random_type].max;
  return min + (h % (max - min + 1));
}

bool SeededRandom::inject_fault(RandomType random_type, uint64_t ctx) {
  uint64_t h = this->hash(random_type, ctx);
  uint64_t max = this->intervals[random_type].max;
  return (h % 1000) < max;
}
