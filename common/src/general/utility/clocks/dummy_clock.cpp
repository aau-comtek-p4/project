#include "general/interfaces/utility/clocks/dummy_clock.h"
void DummyClock::setup() {}
uint64_t DummyClock::rt_now_ns() { return 0; }
uint64_t DummyClock::rt_now_ms() { return 0; }

uint64_t DummyClock::rt_since_start_ms() { return 0; };
uint64_t DummyClock::rt_since_start_ns() { return 0; };
uint64_t DummyClock::time_until_tick() { return 0; };
void DummyClock::tick_catchup() {}
uint64_t DummyClock::tick_now() { return 0; };
uint64_t DummyClock::tick() { return 0; };
uint64_t DummyClock::ms_pr_tick() { return 0; }
uint64_t DummyClock::ms_to_tick(uint64_t time_ms) { return 0; }
