#ifndef NULL_RANDOM_H
#define NULL_RANDOM_H

#include "general/interfaces/simulator/random.h"

class NullRandom : public RandomInterface {
public:
  uint64_t inject_value(RandomType random_type, uint64_t ctx) override;
  bool inject_fault(RandomType random_type, uint64_t ctx) override;
};

#endif
