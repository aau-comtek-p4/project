#ifndef RANDOM_INTERFACE_H
#define RANDOM_INTERFACE_H

#include <cstdint>

#define RANDOM_TYPE_AMOUNT 5

enum RandomType {
  NETWORK_LATENCY = 0,
  MISSED_TICK = 1,
  IO_LATENCY = 2,
  PACKET_DROP = 3,
  MISSED_TICK_CHANCE = 4,
};

class RandomInterface {
public:
  virtual uint64_t inject_value(RandomType random_type, uint64_t ctx) = 0;
  virtual bool inject_fault(RandomType random_type, uint64_t ctx) = 0;
};

#endif
