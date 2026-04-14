#include "general/awaiters/sleep_for.h"
#include "general/misc/names.h"
#include <cstdio>

Task<int> sleep_for_routine() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_SLEEP_ROUTINE);
  self_ctxt->trace.start();
  self_ctxt->trace.suspend_trace();
  co_return 0;
}
SkipAwaiter sleep_for(uint64_t timeout_tick) {
  return SkipAwaiter(timeout_tick);
}
