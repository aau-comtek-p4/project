#include "general/awaiters/sleep_for.h"

SkipAwaiter sleep_for(uint64_t timeout) { return SkipAwaiter(timeout); }
