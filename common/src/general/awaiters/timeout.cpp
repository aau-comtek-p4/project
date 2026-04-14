#include "general/awaiters/timeout_awaiter.h"
#include "general/common.h"
#include "general/interfaces/utility/logger.h"

Task<int> timeout_routine(const void *cancel_data) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(9);
  self_ctxt->trace->start();

  if (!self_ctxt->cancelled) {
    self_ctxt->trace->suspend_trace();
    co_return 0;
  }
  self_ctxt->trace->suspend_trace();
  co_return 0;
}
