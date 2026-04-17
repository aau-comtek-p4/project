#include "general/interfaces/utility/clocks/dummy_clock.h"
uint64_t DummyClock::rt_now_ns() { return 0; }
uint64_t DummyClock::rt_now_ms() { return 0; }
uint64_t DummyClock::rt_since_start_ms() { return 0; };
uint64_t DummyClock::rt_since_start_ns() { return 0; };
